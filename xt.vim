" Vim syntax file
" Language:	Diff (context or unified)
" Maintainer:	Bram Moolenaar <Bram@vim.org>
" Last Change:	2003 Apr 02

" For version 5.x: Clear all syntax items
" For version 6.x: Quit when a syntax file was already loaded
if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

syn match begin         "^TRACE START"
syn match end           "^TRACE END"
syn match date          "\[.*\]"

syn match min_memory        "+\d\+"
syn match pls_memory        "-\d\+"
syn match nll_memory    "+0"

syn match level         "->"
syn match lineno        ":\d\+$"


syn match methodcall      "\k\+->"
syn match staticcall      "\k\+::"
syn match functionb       "\k\+("
syn match functione       ") "

syn match main            "{main}()"
syn match include         "include\(_once\)\=('.\+')"

" Define the default highlighting.
" For version 5.7 and earlier: only when not done already
" For version 5.8 and later: only when an item doesn't have highlighting yet
if version >= 508 || !exists("did_diff_syntax_inits")
  if version < 508
    let did_diff_syntax_inits = 1
    command -nargs=+ HiLink hi link <args>
  else
    command -nargs=+ HiLink hi def link <args>
  endif

  HiLink begin	Label
  HiLink end	Label
  HiLink date   Label

  HiLink level  SpecialChar

  HiLink min_memory Constant
  HiLink pls_memory Structure
  HiLink nll_memory Comment

  HiLink main       Structure
  HiLink include    Structure
  
  HiLink lineno Delimiter
  HiLink methodcall Function
  HiLink staticcall Function
  HiLink functionb  Function
  HiLink functione  Function

  delcommand HiLink
endif

let b:current_syntax = "xt"

" vim: ts=8 sw=2
