#include "cat.h"
#include "platform.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAT_BUFSIZE                  1024
#define CAT_LINENO_ADJUST            6
#define CAT_LINENO_ADJUST_LONGOPT    24
#define CAT_PROGNAME                 "cat"
#define CAT_VERSION                  "0.0.3"

enum cat_command_option_enum {
    CAT_OPTION_HELP,
    CAT_OPTION_VERSION,
    CAT_OPTION_PRINT_LINENUM,
    CAT_OPTION_PRINT_LINENUM_NONBLANK,
    CAT_OPTION_SHOW_NONPRINTING,
    CAT_OPTION_SQUEEZE_BLANK,
    CAT_OPTION_COUNT
};

struct cat_command_option {
    char *shortname;
    char *longname;
    char *description;
};

struct cat_app_context {
    bool option_enabled[CAT_OPTION_COUNT];
    char buffer[CAT_BUFSIZE];
    char **files;
    int files_count;
};

static const struct cat_command_option cat_options[CAT_OPTION_COUNT] = {
    [CAT_OPTION_HELP] = {
        .shortname = "-h",
        .longname = "--help",
        .description = "Print this help and exit",
    },
    [CAT_OPTION_VERSION] = {
        .shortname = "-V",
        .longname = "--version",
        .description = "Print version information and exit",
    },
    [CAT_OPTION_PRINT_LINENUM] = {
        .shortname = "-n",
        .longname = "--number",
        .description = "Number all output lines",
    },
    [CAT_OPTION_PRINT_LINENUM_NONBLANK] = {
        .shortname = "-b",
        .longname = "--number-nonblank",
        .description = "Number nonempty output lines, overrides -n",
    },
    [CAT_OPTION_SQUEEZE_BLANK] = {
        .shortname = "-s",
        .longname = "--squeeze-blank",
        .description = "Suppress repeated empty output lines",
    },
    [CAT_OPTION_SHOW_NONPRINTING] = {
        .shortname = "-v",
        .longname = "--show-nonprinting",
        .description = "Use ^ and M- notation, except for LFD and TAB (not implemented yet)",
    }
};

static const char *invalid_option;

/*
 * Static methods and their declarations.
 */

static void cat_check_ptr_on_init(const void *ptr);
static void cat_print_help(void);
static void cat_print_version(void);
static void cat_print_invalid_option(void);
static void cat_process_files(struct cat_app_context *ctx);
static void cat_process_file(struct cat_app_context *ctx, FILE *fp);
static void cat_parse_options(struct cat_app_context *ctx, int argc, char *argv[]);
static void cat_parse_long_option(struct cat_app_context *ctx, const char *option);
static void cat_parse_short_option(struct cat_app_context *ctx, const char *option);

/*
 * Definition of methods in the header file 'cat.h'.
 */

struct cat_app_context *cat_app_context_init(int argc, char *argv[]) {
    struct cat_app_context *ctx = malloc(sizeof(struct cat_app_context));
    cat_check_ptr_on_init(ctx);
    memset(ctx, 0, sizeof(struct cat_app_context));
    ctx->files = malloc((argc - 1) * sizeof(char *));
    cat_check_ptr_on_init(ctx->files);
    cat_parse_options(ctx, argc, argv);

    return ctx;
}

void cat_run(struct cat_app_context *context) {
    if (invalid_option) {
        cat_print_invalid_option();
        return;
    }
    if (context->option_enabled[CAT_OPTION_HELP]) {
        cat_print_help();
        return;
    }
    if (context->option_enabled[CAT_OPTION_VERSION]) {
        cat_print_version();
        return;
    }
    cat_process_files(context);
}

void cat_app_context_destroy(struct cat_app_context *context) {
    free(context->files);
    free(context);
}

/*
 * Static methods and their definitions
 */

/**
 * Checks whether a pointer is NULL or not on context initialization. If it is, print and exit.
 * @param ptr Pointer to check if NULL.
 */
static void cat_check_ptr_on_init(const void *ptr) {
    if (ptr == NULL) {
        fprintf(stderr, "%s: Failed on context initialization.\n", CAT_PROGNAME);
        exit(EXIT_FAILURE);
    }
}

/**
 * Prints help message.
 */
static void cat_print_help(void) {
    int i;
    fprintf(stdout, "%s\n\n", CAT_PROGNAME);
    fprintf(stdout, "Print and concatenate files.\n");
    for (i = 0; i < CAT_OPTION_COUNT; i++) {
        printf("  %-*s  %-*s  %s\n",
               CAT_LINENO_ADJUST, cat_options[i].shortname,
               CAT_LINENO_ADJUST_LONGOPT, cat_options[i].longname,
               cat_options[i].description);
    }
}

/**
 * Prints version information.
 */
static void cat_print_version(void) {
    fprintf(stdout, "%s %s\n", CAT_PROGNAME, CAT_VERSION);
}

/**
 * Prints if an invalid option is provided.
 */
static void cat_print_invalid_option(void) {
    fprintf(stderr, "%s: Invalid option -- '%s'\n", CAT_PROGNAME, invalid_option);
    fprintf(stderr, "Try 'cat --help' to get help message.");
}

/**
 * Parses command-line arguments and sets up application parameters (options and input files) accordingly.
 * @param ctx Application context
 * @param argc Argument count
 * @param argv Argument vector
 */
static void cat_parse_options(struct cat_app_context *ctx, int argc, char *argv[]) {
    int i;
    char *shortarg;
    argc--;
    argv++;
    for (i = 0; i < argc; i++) {
        if (strncmp(argv[i], "--", 2) == 0) {
            /* Parse long option */
            cat_parse_long_option(ctx, argv[i]);
        } else if (strcmp(argv[i], "-") == 0) {
            /* Reading stdin */
            ctx->files[ctx->files_count] = NULL;
            ctx->files_count++;
        } else if (argv[i][0] == '-') {
            /* Parse short option(s) like -abc or -a */
            shortarg = argv[i] + 1;
            cat_parse_short_option(ctx, shortarg);
        } else {
            /* Add filename to filelist */
            ctx->files[ctx->files_count] = argv[i];
            ctx->files_count++;
        }
    }
}

/**
 * Checks provided long option is in the application's option set.
 * If it is the case, sets the relevant option enabled.
 * @param ctx Application context
 * @param option Long option
 */
static void cat_parse_long_option(struct cat_app_context *ctx, const char *option) {
    int i;
    int valid_op = 0;
    for (i = 0; i < CAT_OPTION_COUNT; i++) {
        if (strcmp(option, cat_options[i].longname) == 0) {
            ctx->option_enabled[i] = true;
            valid_op = 1;
            break;
        }
    }
    if (valid_op == 0) {
        invalid_option = option;
    }
}

/**
 * Checks provided short option(s) is/are in the application's option set.
 * If it is the case, sets the relevant option(s) enabled.
 * @param ctx Application context
 * @param option Short option(s)
 */
static void cat_parse_short_option(struct cat_app_context *ctx, const char *option) {
    size_t i, j;
    int valid_op;
    size_t option_len = strlen(option);
    for (i = 0; i < option_len; i++) {
        valid_op = 0;
        for (j = 0; j < CAT_OPTION_COUNT; j++) {
            if (option[i] == cat_options[j].shortname[1]) {
                ctx->option_enabled[j] = true;
                valid_op = 1;
                break;
            }
        }
        if (valid_op == 0) {
            invalid_option = option;
        }
    }
}

/**
 * Processes input files.
 * @param ctx Application context
 */
static void cat_process_files(struct cat_app_context *ctx) {
    int i;
    if (ctx->files_count == 0) {
        cat_process_file(ctx, stdin);
    }
    for (i = 0; i < ctx->files_count; i++) {
        if (ctx->files[i] == NULL) {
            cat_process_file(ctx, stdin);
            continue;
        }
        FILE *fp = fopen(ctx->files[i], "r");
        if (fp == NULL) {
            fprintf(stderr, "%s: %s: %s\n", CAT_PROGNAME, ctx->files[i], strerror(errno));
            continue;
        }
        if (cat_is_directory(ctx->files[i])) {
            fprintf(stderr, "%s: %s: %s\n", CAT_PROGNAME, ctx->files[i], strerror(EISDIR));
            fclose(fp);
            continue;
        }
        cat_process_file(ctx, fp);
        fclose(fp);
    }
}

/**
 * Process input file and formats it based on provided options.
 * @param ctx Application context
 * @param fp File pointer
 */
static void cat_process_file(struct cat_app_context *ctx, FILE *fp) {
    int linenum = 1;
    int blank_line_repeated = 0;
    int line_continued = 0;
    char *format = "%s";

    if (ctx->option_enabled[CAT_OPTION_PRINT_LINENUM] ||
        ctx->option_enabled[CAT_OPTION_PRINT_LINENUM_NONBLANK]) {
        format = "%*d  %s";
    }

    while (fgets(ctx->buffer, CAT_BUFSIZE, fp)) {
        size_t buf_len = strlen(ctx->buffer);
        if (ctx->option_enabled[CAT_OPTION_SQUEEZE_BLANK]) {
            if (buf_len > 1) {
                blank_line_repeated = 0;
            } else {
                blank_line_repeated++;
            }
        }
        if (ctx->option_enabled[CAT_OPTION_PRINT_LINENUM_NONBLANK]) {
            if (buf_len > 1) {
                if (line_continued) {
                    fprintf(stdout, "%s", ctx->buffer);
                } else {
                    fprintf(stdout, format, CAT_LINENO_ADJUST, linenum, ctx->buffer);
                }
                if (ctx->buffer[buf_len - 1] == '\n') {
                    linenum++;
                    line_continued = 0;
                } else {
                    line_continued = 1;
                }
            } else {
                if (blank_line_repeated <= 1) {
                    fprintf(stdout, "%s", ctx->buffer);
                }
            }
        } else if (ctx->option_enabled[CAT_OPTION_PRINT_LINENUM]) {
            if (blank_line_repeated <= 1) {
                if (line_continued) {
                    fprintf(stdout, "%s", ctx->buffer);
                } else {
                    fprintf(stdout, format, CAT_LINENO_ADJUST, linenum, ctx->buffer);
                }
                if (ctx->buffer[buf_len - 1] == '\n') {
                    linenum++;
                    line_continued = 0;
                } else {
                    line_continued = 1;
                }
            }
        } else {
            if (blank_line_repeated <= 1) {
                fprintf(stdout, "%s", ctx->buffer);
            }
        }
    }
}
