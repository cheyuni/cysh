#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <dirent.h>

#define MAX 1024
#define SMALL_MAX 256
#define PIPE_MAX 10
void fatal(char * msg);
int print_prompt();
void init();
int set_params(char * buf, char * params[MAX]);
void parse_and_eval();
void div_ele();
void eval();
void eval_pipe(char * argu_point[], int pipe_index, int async);
int eval_cd(char ** params, int count);
void redirect(char **params, int index, int is_ltlt);
void run();
enum is {PIPE, GT, GTGT, LT, LTLT, GTBANG, NOR};
char * his_dir;
