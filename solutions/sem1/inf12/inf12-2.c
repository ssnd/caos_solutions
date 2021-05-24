#include "stdio.h"
#include <fcntl.h>
#include <memory.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * Structs
 */
struct RequestData {
    char* method;
    char* host;
    char* script_path;
    char* script_name;
    char* query_string;
};

/** Helper functions **/
char* GetSubstring(const char input[], int l, int r)
{
    char* result = calloc(r - l, sizeof(char));
    int counter = 0;
    for (int i = l; i < r; ++i, ++counter) {
        result[counter] = input[i];
    }
    result[counter] = '\0';
    return result;
}

int FindCharIndex(char c, char* input)
{
    char* b = strchr(input, c);
    return b - input;
}

int FindCharCount(char c, char* input)
{
    int count = 0;
    for (int i = 0; i < strlen(input); ++i)
        if (input[i] == c)
            ++count;

    return count;
}

char* ReturnEnvvarString(char key[], char value[])
{
    char* result =
        (char*)malloc(sizeof(char) * (strlen(key) + strlen(value) + 2));
    snprintf(result, strlen(key) + strlen(value) + 2, "%s=%s", key, value);
    return result;
}

char* FindBetween(char* ptr, char* before, char after)
{
    char buffer[4096];

    char* host_prefix = strstr(ptr, before);

    while (ptr != host_prefix) {
        ++ptr;
    }
    ptr += strlen(before);
    memset(buffer, '\0', 4096);
    int counter = 0;
    while (*ptr != after) {
        buffer[counter++] = *ptr;
        ++ptr;
    }

    char* host = (char*)calloc(sizeof(char), counter);
    memcpy(host, buffer, counter);
    return host;
}

char* FindScriptPath(char* current_char)
{
    char* script_path = calloc(4096, 1);
    int counter = 0;

    script_path[counter++] = '.';
    script_path[counter++] = '/';

    while (*current_char != ' ')
        ++current_char;
    ++current_char;
    while (*current_char != '?' && *current_char != ' ') {
        script_path[counter++] = *current_char;
        current_char++;
    }

    return script_path;
}

void ExecuteScript(struct RequestData* request_data)
{
    char** envvar_list = (char**)malloc(sizeof(char*) * 5);
    envvar_list[0] = ReturnEnvvarString("HTTP_HOST", request_data->host);
    envvar_list[1] = ReturnEnvvarString("REQUEST_METHOD", request_data->method);
    envvar_list[2] =
        ReturnEnvvarString("QUERY_STRING", request_data->query_string);
    envvar_list[3] =
        ReturnEnvvarString("SCRIPT_NAME", request_data->script_name);
    envvar_list[4] = NULL;

    int fd = open(request_data->script_path, O_RDONLY);
    if (fd == -1) {
        printf("HTTP/1.1 404 ERROR\n\n");
        return;
    }

    struct stat st;
    fstat(fd, &st);
    if (!(st.st_mode & S_IXUSR)) {
        printf("HTTP/1.1 403 ERROR\n\n");
        return;
    }

    printf("HTTP/1.1 200 OK\n");
    fflush(stdout);
    execve(request_data->script_path, NULL, envvar_list);
    printf("Return not expected. Must be an execve error\n");
    exit(1);
}

int main(int argc, char* argv[])
{
    char* a = malloc(sizeof(char) * 8192);

    memset(a, '\0', 8192);
    int ch;
    size_t i = 0;
    while ((ch = getchar()) != EOF) {
        a[i++] = (char)ch;
        if (ch == '\n' && a[i - 2] == '\n')
            break;
    }
    a[i] = '\0';
    char* ptr = &a[0];

    struct RequestData request_data;
    // method
    int method_end_index = FindCharIndex(' ', a);
    char* method = GetSubstring(a, 0, method_end_index);
    request_data.method = method;

    // script path
    char* script_path = FindScriptPath(ptr);
    request_data.script_path = script_path;

    // script name
    char* script_name = strrchr(script_path, '/') + 1;
    request_data.script_name = script_name;

    // query params
    struct HTTPQueryParam* query_params = NULL;
    int py_argc = FindCharCount('=', ptr);

    request_data.query_string = "";

    if (py_argc) {
        // query string
        char* query_string = FindBetween(ptr, "?", ' ');
        request_data.query_string = query_string;
    }

    // http host
    char* http_host = FindBetween(ptr, "Host: ", '\n');
    request_data.host = http_host;

    ExecuteScript(&request_data);
    free(a);

    return 0;
}
