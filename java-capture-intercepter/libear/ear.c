#define _GNU_SOURCE

#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>

extern char **environ;

#define ENV_OUTPUT "INTERCEPT_BUILD_TARGET_DIR"
#define ENV_TOOL_PATH "JAVA_CAPTURE_TOOL_PATH"
#define ENV_SIZE 2

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT "libear: (" __FILE__ ":" TOSTRING(__LINE__) ") "

#define PERROR(msg) do { perror(AT msg); } while (0)

#define ERROR_AND_EXIT(msg) do { PERROR(msg); exit(EXIT_FAILURE); } while (0)

#define DLSYM(TYPE_, VAR_, SYMBOL_)                                 \
    union {                                                         \
        void *from;                                                 \
        TYPE_ to;                                                   \
    } cast;                                                         \
    if (0 == (cast.from = dlsym(RTLD_NEXT, SYMBOL_))) {             \
        PERROR("dlsym");                                            \
        exit(EXIT_FAILURE);                                         \
    }                                                               \
    TYPE_ const VAR_ = cast.to;


typedef char const * bear_env_t[ENV_SIZE];

static int capture_env_t(bear_env_t *env);
static void release_env_t(bear_env_t *env);
static char const **string_array_partial_update(char *const envp[], bear_env_t *env);
static char const **string_array_single_update(char const *envs[], char const *key, char const *value);
static void report_call(char const *const argv[]);
static int write_report(int fd, char const *const argv[]);
static char const **string_array_from_varargs(char const * arg, va_list *args);
static char const **string_array_copy(char const **in);
static size_t string_array_length(char const *const *in);
static void string_array_release(char const **);
static int is_end_with(const char *str1, char *str2);
static int get_len(char *const argv[]);
static char* get_java_agent(char* base_tool_path);


static bear_env_t env_names = { ENV_OUTPUT, ENV_TOOL_PATH };

static bear_env_t initial_env = { 0, 0 };

static int initialized = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void on_load(void) __attribute__((constructor));
static void on_unload(void) __attribute__((destructor));

static int mt_safe_on_load(void);
static void mt_safe_on_unload(void);


static int call_execve(const char *path, char *const argv[],
                       char *const envp[]);
                       static int call_execve(const char *path, char *const argv[],
                       char *const envp[]);

static int call_execvp(const char *file, char *const argv[]);

static int call_execvpe(const char *file, char *const argv[],
                        char *const envp[]);

static int call_execvP(const char *file, const char *search_path,
                       char *const argv[]);

static int call_exect(const char *path, char *const argv[],
                      char *const envp[]);

static void on_load(void) {
    pthread_mutex_lock(&mutex);
    if (0 == initialized)
        initialized = mt_safe_on_load();
    pthread_mutex_unlock(&mutex);
}

static void on_unload(void) {
    pthread_mutex_lock(&mutex);
    if (0 != initialized)
        mt_safe_on_unload();
    initialized = 0;
    pthread_mutex_unlock(&mutex);
}

static int mt_safe_on_load(void) {
    // Capture current relevant environment variables
    return capture_env_t(&initial_env);
}

static void mt_safe_on_unload(void) {
    release_env_t(&initial_env);
}

int execve(const char *path, char *const argv[], char *const envp[]) {
    char const * const java_capture_tool_path = getenv(ENV_TOOL_PATH);

    if (is_end_with(path, "javac")) {
        // todo call command to handle the arguments
        const char *java_capture_collect = "/bin/java-capture-collect";
        size_t const path_max_length = strlen(java_capture_tool_path) + strlen(java_capture_collect) + 1;
        char tool_path[path_max_length];
        if (-1 == snprintf(tool_path, path_max_length, "%s/bin/java-capture-collect", java_capture_tool_path))
            ERROR_AND_EXIT("snprintf");
        int res = call_execve(tool_path, argv, envp);
        if (res != 0) {
            printf("run collect javac command failed");
        }
    }

    if (is_end_with(path, "java")) {
        int len = get_len(argv);
        printf("%d\n", len);
        char *argx[len + 2];
        argx[0] = argv[0];

        char *str = "-javaagent:%s/btrace-2.0.1-bin/libs/btrace-agent.jar=noServer=true,trusted=true,script=%s/script/TraceJavacArgs.class,systemClassPath=%s/lib/com.sun.tools-1.8.0_jdk8u275-b01_linux_x64.jar:%s/lib/hamcrest-core-1.3.jar:%s/lib/java-capture-agent.jar:%s/lib/slf4j-api-1.7.30.jar";
        int java_agent_max_length = strlen(str) + strlen(java_capture_tool_path) * 6 + 1;
        char java_agent[java_agent_max_length];
        if (-1 == snprintf(java_agent, java_agent_max_length, str, java_capture_tool_path, java_capture_tool_path, java_capture_tool_path, java_capture_tool_path, java_capture_tool_path, java_capture_tool_path))
            ERROR_AND_EXIT("snprintf");
        argx[1] = java_agent;

        if (len > 1) {
            for (int i = 1; i < len; i ++) {
                argx[i + 1] = argv[i];
            }
        }
        argx[len + 1] = NULL;

        return call_execve(path, argx, envp);
    }

    // report_call((char const *const *)argv);
    return call_execve(path, argv, envp);
}

int execv(const char *path, char *const argv[]) {
    report_call((char const *const *)argv);
    return call_execve(path, argv, environ);
}

int execvpe(const char *file, char *const argv[], char *const envp[]) {
    report_call((char const *const *)argv);
    return call_execvpe(file, argv, envp);
}

int execvp(const char *file, char *const argv[]) {
    report_call((char const *const *)argv);
    return call_execvp(file, argv);
}

int execvP(const char *file, const char *search_path, char *const argv[]) {
    report_call((char const *const *)argv);
    return call_execvP(file, search_path, argv);
}

int exect(const char *path, char *const argv[], char *const envp[]) {
    report_call((char const *const *)argv);
    return call_exect(path, argv, envp);
}

int execl(const char *path, const char *arg, ...) {
    va_list args;
    va_start(args, arg);
    char const **argv = string_array_from_varargs(arg, &args);
    va_end(args);

    report_call((char const *const *)argv);
    int const result = call_execve(path, (char *const *)argv, environ);

    string_array_release(argv);
    return result;
}

int execlp(const char *file, const char *arg, ...) {
    va_list args;
    va_start(args, arg);
    char const **argv = string_array_from_varargs(arg, &args);
    va_end(args);

    report_call((char const *const *)argv);
    int const result = call_execvp(file, (char *const *)argv);

    string_array_release(argv);
    return result;
}

// int execle(const char *path, const char *arg, ..., char * const envp[]);
int execle(const char *path, const char *arg, ...) {
    va_list args;
    va_start(args, arg);
    char const **argv = string_array_from_varargs(arg, &args);
    char const **envp = va_arg(args, char const **);
    va_end(args);

    report_call((char const *const *)argv);
    int const result =
        call_execve(path, (char *const *)argv, (char *const *)envp);

    string_array_release(argv);
    return result;
}

/**
 * check if a string is end with another string
 * 1: yes
 * 0: no
 * -1: some error occur
 */
static int is_end_with(const char *str1, char *str2)
{
    if(str1 == NULL || str2 == NULL)
        return -1;
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    if((len1 < len2) ||  (len1 == 0 || len2 == 0))
        return -1;
    while(len2 >= 1)
    {
        if(str2[len2 - 1] != str1[len1 - 1])
            return 0;
        len2--;
        len1--;
    }
    return 1;
}

static int get_len(char *const argv[]) {
    int index = 0;
    while (argv[index] != NULL)
    {
        index ++;
    }
    
    return index;
}

static int call_execve(const char *path, char *const argv[],
                       char *const envp[]) {
    typedef int (*func)(const char *, char *const *, char *const *);

    DLSYM(func, fp, "execve");

    char const **const menvp = string_array_partial_update(envp, &initial_env);
    int const result = (*fp)(path, argv, (char *const *)menvp);
    string_array_release(menvp);
    return result;
}

static int call_execvpe(const char *file, char *const argv[],
                        char *const envp[]) {
    typedef int (*func)(const char *, char *const *, char *const *);

    DLSYM(func, fp, "execvpe");

    char const **const menvp = string_array_partial_update(envp, &initial_env);
    int const result = (*fp)(file, argv, (char *const *)menvp);
    string_array_release(menvp);
    return result;
}

static int call_execvp(const char *file, char *const argv[]) {
    typedef int (*func)(const char *file, char *const argv[]);

    DLSYM(func, fp, "execvp");

    char **const original = environ;
    char const **const modified = string_array_partial_update(original, &initial_env);
    environ = (char **)modified;
    int const result = (*fp)(file, argv);
    environ = original;
    string_array_release(modified);

    return result;
}

static int call_execvP(const char *file, const char *search_path,
                       char *const argv[]) {
    typedef int (*func)(const char *, const char *, char *const *);

    DLSYM(func, fp, "execvP");

    char **const original = environ;
    char const **const modified = string_array_partial_update(original, &initial_env);
    environ = (char **)modified;
    int const result = (*fp)(file, search_path, argv);
    environ = original;
    string_array_release(modified);

    return result;
}

static int call_exect(const char *path, char *const argv[],
                      char *const envp[]) {
    typedef int (*func)(const char *, char *const *, char *const *);

    DLSYM(func, fp, "exect");

    char const **const menvp = string_array_partial_update(envp, &initial_env);
    int const result = (*fp)(path, argv, (char *const *)menvp);
    string_array_release(menvp);
    return result;
}

static void report_call(char const *const argv[]) {
    // TODO xx
    FILE *fp;
    char * name[] = {"filen1", "file2", "file3", "file4", "file4"};
    fp = fopen("/mnt/e/wsl/xx/test.txt", "w");
    for (int i = 0; i < 5; i++)
    {
        fprintf(fp, "%s\n", name[i]);
    }
    fclose(fp);
}

static int capture_env_t(bear_env_t *env) {
    for (size_t it = 0; it < ENV_SIZE; ++it) {
        char const * const env_value = getenv(env_names[it]);
        if (0 == env_value) {
            PERROR("getenv");
            return 0;
        }

        char const * const env_copy = strdup(env_value);
        if (0 == env_copy) {
            PERROR("strdup");
            return 0;
        }

        (*env)[it] = env_copy;
    }
    return 1;
}

static void release_env_t(bear_env_t *env) {
    for (size_t it = 0; it < ENV_SIZE; ++it) {
        free((void *)(*env)[it]);
        (*env)[it] = 0;
    }
}

static char const **string_array_partial_update(char *const envp[], bear_env_t *env) {
    char const **result = string_array_copy((char const **)envp);
    for (size_t it = 0; it < ENV_SIZE && (*env)[it]; ++it)
        result = string_array_single_update(result, env_names[it], (*env)[it]);
    return result;
}

static char const **string_array_single_update(char const *envs[], char const *key, char const * const value) {
    // find the key if it's there
    size_t const key_length = strlen(key);
    char const **it = envs;
    for (; (it) && (*it); ++it) {
        if ((0 == strncmp(*it, key, key_length)) &&
            (strlen(*it) > key_length) && ('=' == (*it)[key_length]))
            break;
    }
    // allocate a environment entry
    size_t const value_length = strlen(value);
    size_t const env_length = key_length + value_length + 2;
    char *env = malloc(env_length);
    if (0 == env)
        ERROR_AND_EXIT("malloc");
    if (-1 == snprintf(env, env_length, "%s=%s", key, value))
        ERROR_AND_EXIT("snprintf");
    // replace or append the environment entry
    if (it && *it) {
        free((void *)*it);
        *it = env;
	    return envs;
    } else {
        size_t const size = string_array_length(envs);
        char const **result = realloc(envs, (size + 2) * sizeof(char const *));
        if (0 == result)
            ERROR_AND_EXIT("realloc");
        result[size] = env;
        result[size + 1] = 0;
        return result;
    }
}

static char const **string_array_from_varargs(char const *const arg, va_list *args) {
    char const **result = 0;
    size_t size = 0;
    for (char const *it = arg; it; it = va_arg(*args, char const *)) {
        result = realloc(result, (size + 1) * sizeof(char const *));
        if (0 == result)
            ERROR_AND_EXIT("realloc");
        char const *copy = strdup(it);
        if (0 == copy)
            ERROR_AND_EXIT("strdup");
        result[size++] = copy;
    }
    result = realloc(result, (size + 1) * sizeof(char const *));
    if (0 == result)
        ERROR_AND_EXIT("realloc");
    result[size++] = 0;

    return result;
}

static char const **string_array_copy(char const **const in) {
    size_t const size = string_array_length(in);

    char const **const result = malloc((size + 1) * sizeof(char const *));
    if (0 == result)
        ERROR_AND_EXIT("malloc");

    char const **out_it = result;
    for (char const *const *in_it = in; (in_it) && (*in_it);
         ++in_it, ++out_it) {
        *out_it = strdup(*in_it);
        if (0 == *out_it)
            ERROR_AND_EXIT("strdup");
    }
    *out_it = 0;
    return result;
}

static size_t string_array_length(char const *const *const in) {
    size_t result = 0;
    for (char const *const *it = in; (it) && (*it); ++it)
        ++result;
    return result;
}

static void string_array_release(char const **in) {
    for (char const *const *it = in; (it) && (*it); ++it) {
        free((void *)*it);
    }
    free((void *)in);
}
