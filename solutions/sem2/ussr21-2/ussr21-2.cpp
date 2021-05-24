#include <iostream>
#include <cassert>
#include <stdlib.h>
#include <functional>
#include <dlfcn.h>
#include <string>


typedef void(*constructor_t)(void*);


struct ClassImpl {
  ClassImpl() {};

  ClassImpl(constructor_t constr) : constr_(constr) {};

  void* newInstanceWithSize(size_t sizeofClass) {
    void* ptr = malloc(sizeofClass);
    constr_(ptr);
    return ptr;
  }

private:
  constructor_t constr_;
};

class AbstractClass
{
    friend class ClassLoader;
public:
    explicit AbstractClass();
	  AbstractClass(constructor_t constr) : pImpl(new ClassImpl(constr)) {};
    ~AbstractClass();
protected:

    void* newInstanceWithSize(size_t sizeofClass);
    struct ClassImpl* pImpl;
};

template <class T>
class Class
        : public AbstractClass
{
public:
    T* newInstance()
    {
        size_t classSize = sizeof(T);
        void* rawPtr = newInstanceWithSize(classSize);
        return reinterpret_cast<T*>(rawPtr);
    }
};

enum class ClassLoaderError {
    NoError = 0,
    FileNotFound,
    LibraryLoadError,
    NoClassInLibrary
};


class ClassLoader
{
public:
    explicit ClassLoader();
    AbstractClass* loadClass(const std::string &fullyQualifiedName);
    ClassLoaderError lastError() const;
    ~ClassLoader();
private:
    struct ClassLoaderImpl* pImpl;
};


AbstractClass::AbstractClass() : pImpl(new ClassImpl) {

};

AbstractClass::~AbstractClass() {
	delete pImpl;
};




struct ClassLoaderImpl  {
	ClassLoaderError err{0};
	void * lib_handle{nullptr};

	ClassLoaderImpl() = default;
	ClassLoaderError LastError() const {
		return err;
	}

	~ClassLoaderImpl() {
		DLClose();
	};
	std::string MangleConstructorName(const std::string& name) {
		std::string result_str;
		result_str += "_ZN";

		size_t count = 0;
		std::string buff;
		for (size_t i = 0 ; i < name.length(); ++i) {
			if ( ':' == name[i] && name[i+1] == ':' ) {
				result_str += std::to_string(count);
				result_str += buff;
				count = 0;
				buff = "";
				i+=1;
				continue;
			}

			count+=1;
			buff += name[i];
		}

		result_str += std::to_string(count);
		result_str += buff;
		result_str += "C1Ev";
		return result_str;
	}
	bool FindSOFile(const std::string& fqn) {
		FILE* fptr;
		fptr = fopen(fqn.c_str(),"r");
		if (fptr==NULL) {
			return false;
		}
		fclose(fptr);

		return true;
	}

	std::string FindPathToFQN(const std::string& fqn) {
		std::string result_str(std::getenv("CLASSPATH"));
		result_str += "/";

		for (size_t i = 0; i<fqn.length(); ++i) {
			if (':' ==fqn[i] && ':' ==fqn[i+1]) {
				result_str+="/";
				i+=2;
			}

			result_str+=fqn[i];
		}

		result_str+=".so";

		return result_str;
	}

	std::optional<constructor_t> Constructor(const std::string & mangled) {
		assert(lib_handle != nullptr);

		auto constructor  = reinterpret_cast<constructor_t>(dlsym(lib_handle,mangled.c_str()));
		if (constructor == nullptr) {
			return std::nullopt;
		}
		return constructor;

	}

	AbstractClass * LoadClass(const std::string & fqn) {
		std::string dl_path = FindPathToFQN(fqn);
		if (!FindSOFile(dl_path)) {
			err = ClassLoaderError::FileNotFound;
			return nullptr;
		}
		lib_handle = dlopen(dl_path.c_str(), RTLD_NOW);
		if (!lib_handle) {
			err = ClassLoaderError::LibraryLoadError;
			return nullptr;
		}
		std::string mangled_name = MangleConstructorName(fqn);

		void* entry = dlsym(lib_handle, mangled_name.c_str());
		if (!entry) {
			dlclose(lib_handle);
			err = ClassLoaderError::NoClassInLibrary;
			return nullptr;
		}

		err = ClassLoaderError::NoError;

		return new AbstractClass((constructor_t)entry);
	};

	void DLClose() {
		if (lib_handle != nullptr)
			dlclose(lib_handle);
	};
} ;

ClassLoader::ClassLoader(): pImpl( new ClassLoaderImpl()) {

}

ClassLoaderError ClassLoader::lastError() const {
	return pImpl->LastError();
}

ClassLoader::~ClassLoader() {
	delete pImpl;
}

AbstractClass * ClassLoader::loadClass(const std::string& fqn) {
	return pImpl->LoadClass(fqn);
}

void * AbstractClass::newInstanceWithSize(size_t sizeofClass ) {
	return pImpl->newInstanceWithSize(sizeofClass);
}


