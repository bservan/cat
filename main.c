#include "cat.h"

static struct cat_app_context *context;

int main(int argc, char * argv[]) {
    context = cat_app_context_init(argc, argv);
    cat_run(context);
    cat_app_context_destroy(context);
    return 0;
}
