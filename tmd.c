/*! 2021 (c) Andrea Giammarchi */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Special shell/terminal chars that are *mostly* cross platform
#define BULLET "•"
#define CLREOL "\x1B[K"
#define RESET "\x1b[0m"
#define QUOTE "\x1B[100m \x1B[49m"

// Open/Close parts
#define BOLD_START "\x1b[1m"
#define BOLD_END "\x1b[22m"
#define CODE_START "\x1b[97;100m"
#define CODE_END "\x1b[39;49m"
#define HEADER_START "\x1B[7m"
#define HEADER_END "\x1B[27m"
#define STRIKE_START "\x1B[9m"
#define STRIKE_END "\x1B[29m"
#define UNDERLINE_START "\x1B[4m"
#define UNDERLINE_END "\x1B[24m"

// OS dependent variants
#ifdef _WIN32
#define DIM_START "\x1b[90m"
#define DIM_END "\x1b[37m"
#define NEW_LINE "\r\n"
#elif _WIN64
#define DIM_START "\x1b[90m"
#define DIM_END "\x1b[37m"
#define NEW_LINE "\r\n"
#else
#define DIM_START "\x1b[2m"
#define DIM_END "\x1b[22m"
#define NEW_LINE "\n"
#endif

// Process a buffer and output markdown
void markdown(int length, char *buffer, int nested);

// Reveals spaces and new lines
int is_new_line(char c);
int is_inline_space(char c);
int is_space(char c);

// String utils
int index_of(const char *buffer, int start, int length, const char *chunk, int len, int break_on_line);
void multiline(const char *buffer);
char *slice(const char *str, int start, int end);
char *str(int length);
char *trim(const char *buffer, int length);
char *trim_start(const char *buffer, int length);
char *trim_end(const char *buffer, int length);

// Shortcuts
void out_of_memory(void);
void show(const char *buffer, int start, int end);
void special_char(char c, int open);
void tmd(char *buffer);

// Main program: cat file | tdm or tdm file
int main(int argc, char *argv[])
{
    int i = 0;
    int size = 1024 * sizeof(char);

    // try to allocate enough memory, but not too much
    char *buffer = str(size);

    // if the string was passed via pipe
    if (argc == 1 && !isatty(fileno(stdin)))
    {
        do
        {
            char c = getchar();
            if (c < 0)
            {
                break;
            }
            buffer[i++] = c;
            // increase only when/if necessary
            if (i == size)
            {
                size += size;
                buffer = (char *) realloc(buffer, (size + 1) * sizeof(char));
                // exit if the operation failed
                if (buffer == NULL)
                {
                    out_of_memory();
                }
            }
        }
        while (1);
    }
    // else if there is a file to parse
    else if (argc == 2)
    {
        // guard against non existent files
        FILE *fd = fopen(argv[1], "r");
        if (fd == NULL)
        {
            markdown(strlen(argv[1]), argv[1], 0);
            fprintf(stdout, "%s\n", RESET);
            return 0;
        }
        else
        {
            // read until the end
            do
            {
                char c = fgetc(fd);
                if (c == EOF)
                {
                    break;
                }
                buffer[i++] = c;
                // realloc if needed
                if (i == size)
                {
                    size += size;
                    buffer = (char *) realloc(buffer, (size + 1) * sizeof(char));
                    if (buffer == NULL)
                    {
                        out_of_memory();
                    }
                }
            }
            while (1);
        }
        fclose(fd);
    }
    // else: wrong number of arguments; output an "how to"
    // using the software itself to explain it (meta, isn't it?)
    else
    {
        char *how_to = "\n\
# Tiny Markdown        \n\
\n\
 *usage*\n\
\n\
```\n\
  tmd 'some *markdown*'\n\
  tmd file.md\n\
  cat file.md | tmd\n\
```\n\
\n\
 -by Andrea Giammarchi-\n\
\n";
        tmd(how_to);
        free(buffer);
        return 0;
    }

    // mark the end of the buffer and process
    buffer[i] = '\0';
    markdown(i, buffer, 0);
    free(buffer);

    // ensure the layout is cleaned up after
    fprintf(stdout, "%s", RESET);
    return 0;
}

int index_of(const char *buffer, int start, int length, const char *chunk, int len, int break_on_line)
{
    char *buff = str(len);
    for (int i = start; i < length; i++)
    {
        int end = i + len;
        if (end > length || (break_on_line != 0 && is_new_line(buffer[i]) == 1))
        {
            free(buff);
            return -1;
        }
        // put next N chars in the buffer
        for (int k = 0, j = i; j < end; j++)
        {
            buff[k++] = buffer[j];
        }
        // return the index if it's the same string
        if (strcmp(chunk, buff) == 0)
        {
            free(buff);
            return i;
        }
    }
    free(buff);
    return -1;
}

int is_new_line(char c)
{
    switch (c)
    {
        case '\n':
        case '\r':
            return 1;
    }
    return 0;
}

int is_inline_space(char c)
{
    switch (c)
    {
        case ' ':
        case '\t':
            return 1;
    }
    return 0;
}

int is_space(char c)
{
    switch (c)
    {
        case '\f':
        case '\v':
            return 1;
    }
    return is_inline_space(c) || is_new_line(c);
}

void markdown(int length, char *buffer, int nested)
{
    int start = 0;
    for (int i = 0; i < length; i++)
    {
        char current = buffer[i];
        switch (current)
        {
            // [tite](links)
            case '[':
            {
                int end = index_of(buffer, i + 1, length, "]", 1, 1);
                if (-1 < end && !is_space(buffer[end - 1]) && end < length && buffer[end + 1] == '(')
                {
                    int link = index_of(buffer, end + 2, length, ")", 1, 1);
                    if (-1 < link)
                    {
                        show(buffer, start, i);
                        char *title = slice(buffer, i + 1, end);
                        char *a = slice(buffer, end + 2, link);
                        fprintf(stdout, "%s %s→ %s%s", title, DIM_START, a, DIM_END);
                        free(title);
                        free(a);
                        start = link + 1;
                        i = start - 1;
                    }
                }
                break;
            }
            // > quotes
            case '>':
            {
                // bail out in nested parsing
                if (nested)
                {
                    break;
                }
                // it should be at the beginning of the line
                if (i == 0 || is_new_line(buffer[i - 1]))
                {
                    int code = 1;
                    int j = i + 1;
                    while (j < length && buffer[j] == '>' || is_inline_space(buffer[j]))
                    {
                        j++;
                        code++;
                    }
                    // remove trailing spaces
                    while (is_inline_space(buffer[j - 1]))
                    {
                        j--;
                    }
                    // there should be a space after anyway
                    if (i < j && j < length && is_inline_space(buffer[j]))
                    {
                        show(buffer, start, i);
                        for (int k = i; k < j; k++)
                        {
                            // ignore spaces in between `> >`
                            // so that both `>>` and `> >` results the same
                            if (!is_inline_space(buffer[k]))
                            {
                                fprintf(stdout, "%s", QUOTE);
                            }
                        }
                        start = j;
                        i = start - 1;
                    }
                }
                break;
            }
            // # Headers
            case '#':
            {
                // bail out in nested parsing
                if (nested)
                {
                    break;
                }
                // be sure this is the beginning of a new line
                if (i > 0 && !is_new_line(buffer[i - 1]))
                {
                    break;
                }
                int code = 1;
                int j = i + 1;
                while (j < length && buffer[j] == current)
                {
                    j++;
                    code++;
                }
                // if there is no space after, get out
                if (j < length && !is_space(buffer[j]))
                {
                    break;
                }
                // find the new line to define the header
                int nl = j;
                while (nl < length && !is_new_line(buffer[nl]))
                {
                    nl++;
                }
                show(buffer, start, i);

                char *chunk = slice(buffer, j, nl);
                // # Bold Header + new line
                if (code == 1)
                {
                    fprintf(stdout, "%s%s%s %s%s%s", HEADER_START, BOLD_START, chunk, BOLD_END, HEADER_END, NEW_LINE);
                }
                // ## Other headers + new line
                else
                {
                    fprintf(stdout, "%s%s %s%s", HEADER_START, chunk, HEADER_END, NEW_LINE);
                }
                free(chunk);

                // drop any extra trailing new line after headers
                // as one new line has been added already
                while (nl < length && is_new_line(buffer[nl]))
                {
                    nl++;
                }
                start = nl - 1;
                i = start - 1;
                break;
            }
            // *bold* (maybe * list), _underline_, ~strike~, -dim-, `code`
            case '*':
            case '_':
            case '~':
            case '-':
            case '`':
            {
                // ignore chars that are not meant to wrap words
                if (i > 0 && !is_space(buffer[i - 1]) && current != '`')
                {
                    break;
                }
                // find occurrences of the current char, so that
                // *bold* and **bold** equally works
                int code = 1;
                int j = i + 1;
                while (j < length && buffer[j] == current)
                {
                    j++;
                    code++;
                }
                // multiline code is a special case
                // ```pl
                // code
                // ```
                if (code > 1 && current == '`')
                {
                    while (j < length && !is_new_line(buffer[j]))
                    {
                        j++;
                    }
                    char *chunk = slice(buffer, i, i + code);
                    int end = index_of(buffer, j, length, chunk, code, 0);
                    free(chunk);
                    if (-1 < end)
                    {
                        // normalize new lines before the code by removing trailing new lines
                        char *chunk = slice(buffer, start, i);
                        char *trimmed = trim_end(chunk, i - start);
                        // and if too many were removed, add back one
                        if ((i - start) - strlen(trimmed) > 1)
                        {
                            fprintf(stdout, "%s%s%s", trimmed, NEW_LINE, NEW_LINE);
                        }
                        // or leave it as it is
                        else
                        {
                            fprintf(stdout, "%s%s", trimmed, NEW_LINE);
                        }
                        free(chunk);
                        free(trimmed);

                        // drop trailing new lines at the beginning of the code
                        while (j < length && is_new_line(buffer[j]))
                        {
                            j++;
                        }
                        chunk = slice(buffer, j, end);
                        trimmed = trim_end(chunk, end - j);
                        free(chunk);

                        multiline(trimmed);
                        free(trimmed);

                        start = end + code;
                        i = start - 1;
                    }
                }
                // but all other inline cases are the same
                else if (j < length && !is_space(buffer[j]))
                {
                    char *chunk = slice(buffer, i, i + code);
                    int k = j;
                    while (1)
                    {
                        // find the next inline counter-char(s)
                        int end = index_of(buffer, k, length, chunk, code, 1);
                        // if none, get out
                        if (end < 0)
                        {
                            break;
                        }
                        // else if is not a space, keep searching ...
                        if (is_space(buffer[end - 1]))
                        {
                            k = end + 1;
                        }
                        // found it! substitute it
                        else
                        {
                            show(buffer, start, i);
                            special_char(current, 1);
                            // show nested content as is
                            if (current == '`')
                            {
                                show(buffer, j, end);
                            }
                            // otherwise keep parsing for combined cases such as:
                            // *_bold underlined_*
                            else
                            {
                                char *nested = slice(buffer, j, end);
                                markdown(end - j, nested, 1);
                                free(nested);
                            }
                            special_char(current, 0);
                            start = end + code;
                            i = start - 1;
                            break;
                        }
                    }
                    free(chunk);
                }
                // this is a bail out from *bold* but it could be a list
                else if (current == '*' && !nested)
                {
                    j = i;
                    // look backward for new lines
                    while (j--)
                    {
                        if (!is_inline_space(buffer[j]))
                        {
                            break;
                        }
                    }
                    // if the last non inline space is a new line, we have a match!
                    if (is_new_line(buffer[j]))
                    {
                        // show the right indentation
                        while (is_inline_space(buffer[j]))
                        {
                            fprintf(stdout, "%c", buffer[j]);
                            j++;
                        }
                        show(buffer, start, i);
                        fprintf(stdout, "%s", BULLET);
                        start = i + 1;
                    }
                }
                break;
            }
        }
    }
    show(buffer, start, length);
}

// this whole process is to grant that multi line code
// background color actually spreads across the whole terminal
void multiline(const char *buffer)
{
    int start = 0;
    int last_line = 0;
    int len = strlen(buffer);
    char *chunk;
    fprintf(stdout, "%s%s", CODE_START, CLREOL);
    for (int i = 0; i < len; i++)
    {
        // remember last line to clean up
        if (is_new_line(buffer[i]))
        {
            last_line = i;
        }
        else if (last_line)
        {
            chunk = slice(buffer, start, last_line);
            fprintf(stdout, "%s%s", chunk, CLREOL);
            free(chunk);
            start = last_line;
            last_line = 0;
        }
    }
    chunk = slice(buffer, start, len);
    fprintf(stdout, "%s%s%s", chunk, CLREOL, CODE_END);
    free(chunk);
}

void out_of_memory(void)
{
    printf("out of memory%s", NEW_LINE);
    exit(EXIT_FAILURE);
}

void special_char(char c, int open)
{
    switch (c)
    {
        case '*':
            fprintf(stdout, "%s", open ? BOLD_START : BOLD_END);
            break;
        case '_':
            fprintf(stdout, "%s", open ? UNDERLINE_START : UNDERLINE_END);
            break;
        case '~':
            fprintf(stdout, "%s", open ? STRIKE_START : STRIKE_END);
            break;
        case '-':
            fprintf(stdout, "%s", open ? DIM_START : DIM_END);
            break;
        case '`':
            fprintf(stdout, "%s", open ? CODE_START : CODE_END);
            break;
    }
}

void show(const char *buffer, int start, int end)
{
    char *chunk = slice(buffer, start, end);
    fprintf(stdout, "%s", chunk);
    free(chunk);
}

char *str(int length)
{
    char *string = (char *) malloc((length + 1) * sizeof(char));
    if (string == NULL)
    {
        out_of_memory();
    }
    string[length] = '\0';
    return string;
}

char *slice(const char *buffer, int start, int end)
{
    char *chunk = str(end - start);
    int j = 0;
    for (int i = start; i < end; ++i)
    {
        chunk[j++] = buffer[i];
    }
    chunk[j] = '\0';
    return chunk;
}

void tmd(char *buffer)
{
    markdown(strlen(buffer), buffer, 0);
}

char *trim(const char *buffer, int length)
{
    char *st = trim_start(buffer, length);
    char *et = trim_end(st, strlen(st));
    free(st);
    return et;
}

char *trim_start(const char *buffer, int length)
{
    int start = 0;
    while (start < length && is_space(buffer[start]))
    {
        start++;
    }
    char *chunk = str(length - start);
    for (int j = 0, i = start; i < length; i++)
    {
        chunk[j++] = buffer[i];
    }
    return chunk;
}

char *trim_end(const char *buffer, int length)
{
    int end = length - 1;
    while (end >= 0 && is_space(buffer[end]))
    {
        end--;
    }
    char *chunk = str(++end);
    for (int i = 0; i < end; i++)
    {
        chunk[i] = buffer[i];
    }
    return chunk;
}
