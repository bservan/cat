#ifndef CAT_CAT_H
#define CAT_CAT_H

#define _CRT_SECURE_NO_WARNINGS

struct cat_app_context;

/**
 * Initializes application variables based on command-line arguments.
 * Parsed command-line arguments and allocates application context.
 * @param argc Argument count
 * @param argv Argument vector
 * @return Application context initialized based on command-line arguments
 */
struct cat_app_context *cat_app_context_init(int argc, char *argv[]);

/**
 * Main procedure of program.
 * Processes inputs based on command-line arguments.
 * @param context Application context
 */
void cat_run(struct cat_app_context *context);

/**
 * Deallocates application context.
 * @param context Application context
 */
void cat_app_context_destroy(struct cat_app_context *context);

#endif /* CAT_CAT_H */
