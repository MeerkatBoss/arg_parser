#include <stdio.h>
#include <string.h>

#include "argparser.h"

static const arg_tag* find_tag(const char* str, const arg_info* info);
static int compare_tag(const char* str, const arg_tag* tag);

int parse_args(int argc, const char *const *argv,
                const arg_info *info, void *params)
{
    if (!argv || !info) return -1;

    if (info->name_handler)
    {
        int name_parsed = info->name_handler(argv, params);
        if (name_parsed < 0)
            return 0;
    }
    argv++;

    int total_parsed = 1;
    while(*argv)
    {
        const arg_tag* tag = find_tag(*argv, info);

        int parsed = 0;
        if (tag == NULL)
            parsed = info->plain_handler(argv, params);
        else
            parsed = tag->callback(++argv, params);

        if (parsed < 0)
            return total_parsed;

        argv += parsed;
        total_parsed += parsed;
    }

    return total_parsed;
}

void print_help(const arg_info* info)
{
    puts(info->help_message);
    putc('\n', stdout);

    const int FLAGS_WIDTH = 24; 
    for (size_t i = 0; i < info->tag_cnt; i++)
    {
        int fill = FLAGS_WIDTH;

        if (!info->tags[i].short_tag && !info->tags[i].long_tag)
            continue;

        fputs("\033[1m", stdout);
        if (info->tags[i].short_tag != '\0')
            printf("-%c", info->tags[i].short_tag);
        else
            fputs("    ", stdout);
        fill -= 2;

        if (info->tags[i].long_tag != NULL)
        {
            if (info->tags[i].short_tag != '\0')
            {
                fputs(", ", stdout);
                fill -= 2;
            }
            fill -= printf("--%s", info->tags[i].long_tag);
        }
        fputs("\033[22m", stdout);
        printf("%*s%s\n", fill, "", info->tags[i].description);
    }
}

static const arg_tag* find_tag(const char* str, const arg_info* info)
{
    if (!str[0]) return NULL;
    
    for (size_t i = 0; i < info->tag_cnt; i++)
        if (compare_tag(str, &info->tags[i]))
            return &info->tags[i];

    return NULL;
}

static int compare_tag(const char* str, const arg_tag* tag)
{
    int n_dashes = 0;

    while(*str == '-')
    {
        str++;
        n_dashes++;
    }

    return
        (n_dashes == 1 && str[1] == '\0' && str[0] == tag->short_tag) ||
        (n_dashes == 2 && strcmp(str, tag->long_tag) == 0);
}

