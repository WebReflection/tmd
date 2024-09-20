#!/usr/bin/env bun

import { cc } from 'bun:ffi';

const {
  symbols: {
    main,
    markdown
  },
} = cc({
  source: './tmd.c',
  symbols: {
    main: {
      args: ['int', 'ptr'],
      returns: 'int',
    },
    markdown: {
      args: ['int', 'ptr', 'int'],
      returns: 'void',
    },
  },
});

const [_, __, ...chunks] = process.argv;

// tmd.c already adds the \0 char after the whole thing
// has been consumed or addressed so this is even easier

// ./tmd.js 'hello *world*'
if (chunks.length) {
  const ptr = Buffer.from(chunks.join(' '));
  markdown(ptr.length, ptr, 0);
}
// ./tmd.js or echo 'hello *world*' | ./tmd.js
else main(process.stdin.isTTY ? 0 : 1);
