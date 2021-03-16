# tmd - Tiny Markdown       

#### Video Demo:               

[On YouTube](https://youtu.be/jLmFxWflcDo)


#### Description:              

[Markdown](https://en.wikipedia.org/wiki/Markdown) is a very simple, and popular, markup language, used to write emails, or documents, like this very same one.

The goal of this project is to bring *the ease of markdown to any terminal*, console, or shell.

Why am I doing this?

  * most *Markdown* parsers are based on _regular expressions_ ( -aka: RegExp- ).
  * *not* all platforms ( -micro controllers, your terminal- ) support RegExp out of the box.
  * RegExp libraries are usually big and memory greedy, not always and option in constrained hardware.

So here there's this tiny markdown parser that works natively, wherever *C* is available, based on a single pass, with recursive calls, for nested parsing, output normalization, and cross platform features, wherever supported.

What kind of challenges did I encounter in writing this software?

  * ensure that all commands actually work cross platform (macOS, Windows, Linux)
  * ensure that nested parts of the content can be recursively parsed within their boundaries
  * ensure that there are no memory leaks whatsoever, despite the huge amount of strings manipulation and operations
  * implements a lot of strings functionalities such as `slice`, `trim_start`, `trim_end`, `trim` combined, and `index_of` with multiple chars
  * try to normalize the outcome in every terminal, including a forced homogenous multiline code background and color
  * understand how piping works in C, beside reading documents as explained during the training
  * keeping performance under control, through a single pass with look ahead and look behind when needed
  * ensure that in the worst case scenario, the layout is cleaned up once the program exits

Who would actually benefit from this project?

  * anyone writing CLI with a desire to provide better documentation within the `--help` flag
  * anyone using CLI most of the time, happy to have a visually enhanced experience
  * anyone using any other programming language with a desire to show better outputs in CLI


## Tests                     

    * *style50* passes
    * *valgrind* passes with or without arguments
    * use `make test` or `make style`, if in CS50 IDE, or `make heap`
    * this file is used itself as a test for this program


## Features                  

  * *bold*, _underlined_, -dimmed-, and, for Linux only, ~striked~ text.

```
    *bold* / **bold**
    _underlined_ / __underlined__
    -dimmed- / --dimmed--
    ~striked~ / ~~striked~~
```

  * inline `code`, but also multiline

````
    inline `code` and multiline:

    ```
    code
    ```
````

  * `# Header 1` or `# Header X` text

```
    # Header 1

    ## Header X
    ### Header X
```

  * **`* bullets`** lists

```
    * bullet 1
    * bullet X
```

  * [links](https://example.com)

```
    [link title](https://example.com)
```

**but also ...**
> quoted text

```
    > quoted
    > > quoted nested
    >> also quoted nested
    > quoted
```


## tmd - How To Use it       

The `./tmd` executable created via `Make` will either parse *piped content* or a specific file.

```sh
# pipe through any content
cat file.md | ./tmd

# or specify a file to parse
./tmd file.md
```

The parsed output will be shown right away.
