#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <map>
#include <cstring>
#include <memory>
#include <vector>
#include <iostream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

bool operator<(const timespec &l, const timespec &r) {
  return l.tv_sec < r.tv_sec ||(l.tv_sec == r.tv_sec && l.tv_nsec < r.tv_nsec);
}

class MFSFile {
  std::string src_;
  fs::directory_entry entry_;
  std::map<std::string, MFSFile> children_;
 public:
  timespec LastWriteTime() const {
    if (strcmp(Path(), "/") ==0) {
      return timespec{};
    }

    struct stat st{};
    stat(Path(), &st);
    return st.st_mtim;
  }
  const char *Path() const {
    return entry_.path().c_str();
  }
  bool IsDirectory() const {
    if (strcmp(Path(), "/") == 0) {
      return true;
    }
    struct stat st{};
    stat(Path(), &st);
    return S_ISDIR(st.st_mode);
  }
  size_t CountSubDirectories() const {
    size_t total = 0;
    for (const auto &it: children_) {
      total += it.second.IsDirectory() ? 1 : 0;
    }
    return total;
  }
  const fs::directory_entry &GetEntry() const {
    return entry_;
  }
  const std::map<std::string, MFSFile> &GetChildren() const {
    return children_;
  }
  const std::string &Src() const {
    return src_;
  }
  MFSFile() = default;
//  fs::file_time_type LastWriteTime() const {
//    return entry_.last_write_time();
//  }
  explicit MFSFile(fs::directory_entry entry) : entry_(entry) {
    src_ = entry.path().string().substr(1);
  }

  template<typename TP>
  std::time_t ToTimeT(TP tp) {
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());
    return system_clock::to_time_t(sctp);
  }

  void DisplayLastWriteTime() {
    char buffer[32];
    auto time = ToTimeT(entry_.last_write_time());
    std::tm *ptm = std::localtime(&time);
    std::strftime(buffer, 32, "%a, %d.%m.%Y %H:%M:%S", ptm);
    std::cout << buffer << std::endl;
  }

  template<typename Iterator>
  MFSFile *FindFileByPath(const std::string &path,
                          Iterator begin,
                          Iterator end) {
    auto it = children_.find(*begin);
    if (it == children_.end()) {
      return nullptr;
    }

    if (end - begin == 1) {
      return &it->second;
    }

    return it->second.FindFileByPath(path, begin + 1, end);
  }

  template<typename Iterator>
  void AddChild(MFSFile file, Iterator begin, Iterator end) {
    if (begin == end) {
      return;
    }

    auto found_file = children_.find(*begin);
    if (found_file == children_.end()) {
      children_[*begin] = MFSFile(file);
    } else if (end - begin == 1 && found_file->second.LastWriteTime() < file.LastWriteTime()) {
      children_[*begin] = MFSFile(file);
    }
    if (end-begin != 1) {
      children_[*begin].AddChild(std::move(file), begin + 1, end);
    }
  }

};

using Path = std::vector<std::string>;

class MFS {
 public:
  MFSFile root_file_{};
  MFS(const char *input_files) {
    auto paths = SplitByChar(input_files, ':');
    for (std::string & path: paths) {
      char * resolved_path = realpath(path.c_str(), nullptr);
      path = std::string(resolved_path, strlen(resolved_path));
      free(resolved_path);
    }

    for (const auto &path: paths) {
      size_t path_start_index = SplitByChar(path, '/').size();
      for (const auto &entry: fs::recursive_directory_iterator(path)) {
        std::string entry_path = entry.path().string().substr(1);
        MFSFile new_file(entry);
        auto path_vector = SplitByChar(entry_path, '/');
        root_file_.AddChild(std::move(new_file),
                            path_vector.begin() + path_start_index - 1,
                            path_vector.end());

      }
    }
  }

  static const std::string PathToString(const Path &path) {
    std::string result;
    for (const auto &it: path) {
      result += it;
    }
    return result;
  }

  static Path SplitByChar(const std::string &input,
                          const char &by) {
    std::vector<std::string> result;
    std::string buff;
    for (const auto &ch: input) {
      if (ch == by) {
        result.emplace_back(std::string(buff));
        buff = "";  // todo: ok?
        continue;
      }
      buff += ch;
    }
    if (buff.size() != 0) {
      result.emplace_back(std::string(buff));
    }
    return result;
  }

  MFSFile *FindFileByPath(const std::string &path) {
    auto splitted_path = SplitByChar(path.substr(1), '/');
    return root_file_.FindFileByPath(path, splitted_path.begin(), splitted_path.end());
  }
};

std::shared_ptr<MFS> mfs;

struct MFSOptions {
  const char *input_folders;
};

struct MFSOperations {
  static int getattr(const char *path,
                     struct stat *st_ptr,
                     struct fuse_file_info *file_info) {
    if (strcmp(path, "/") == 0) {
      *st_ptr = (struct stat) {
        .st_nlink = 2 + mfs->root_file_.CountSubDirectories(),
        .st_mode = S_IFDIR | 0555
      };
      return 0;
    }

    MFSFile *file = mfs->FindFileByPath(path);
    if (!file) {
      return -ENOENT;
    }

    stat(file->Path(), st_ptr);

    if (S_ISDIR(st_ptr->st_mode)) {
      st_ptr->st_nlink = 2 + file->CountSubDirectories();
      st_ptr->st_mode = S_IFDIR | 0555;
      return 0;
    }

    st_ptr->st_nlink = 1;
    st_ptr->st_mode = S_IFREG | 0444;
    return 0;

  }
  static int readdir(const char *path,
                     void *out1,
                     fuse_fill_dir_t filler,
                     off_t off,
                     struct fuse_file_info *fi,
                     enum fuse_readdir_flags flags) {

    filler(out1, ".", nullptr, 0, static_cast<fuse_fill_dir_flags>(0));
    filler(out1, "..", nullptr, 0, static_cast<fuse_fill_dir_flags>(0));

    MFSFile *f{nullptr};

    if (strcmp(path, "/") == 0) {
      f = &mfs->root_file_;
    } else {
      f = mfs->FindFileByPath(path);
    }

    if (!f) {
      return -ENOENT;
    }

    if (strcmp(path, "/") != 0 && !f->IsDirectory()) {
      return -ENOTDIR;
    }

    for (const auto &it: f->GetChildren()) {
      filler(out1, it.first.c_str(), nullptr, 0, static_cast<fuse_fill_dir_flags>(0));
    }

    return 0;
  }

  static int readfile(const char *path,
           char *out,
           size_t size,
           off_t off,
           struct fuse_file_info *fi) {
    if (strcmp(path, "/") == 0) {
      return -EISDIR;
    }

    MFSFile *file = mfs->FindFileByPath(path);
    if (!file) {
      return -ENOENT;
    }

    if (file->IsDirectory()) {
      return -EISDIR;
    }

    int fd = open(file->Path(), O_RDONLY);
    lseek(fd, off, SEEK_SET);
    int32_t bytes = read(fd, out, size);
    close(fd);
    if (bytes == -1) {
      return -EIO;
    }

    return bytes;
  }
};

static MFSOptions options;

struct fuse_operations operations = {
    .getattr=MFSOperations::getattr,
    .read=MFSOperations::readfile,
    .readdir=MFSOperations::readdir,
};

static const struct fuse_opt option_spec[] = {
    {"--src %s", offsetof(MFSOptions, input_folders), 0},
    FUSE_OPT_END
};

int main(int argc, char *argv[]) {
  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

  if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
    return 1;

  mfs = std::make_shared<MFS>(options.input_folders);
  int ret = fuse_main(
      args.argc, args.argv,
      &operations,
      nullptr
  );
  fuse_opt_free_args(&args);
  return ret;
}
