# Day 23 – Scope and Local/Global Variables

**Date:** April 05, 2026  
**Status:** Completed

## Goals
- Understand variable scope (local vs global)
- Learn lifetime and visibility of variables
- Practice proper use of namespaces to avoid global pollution
- Avoid common pitfalls with global variables

## What I Learned / Key Takeaways

- **Local variables**: declared inside a block/function, visible only there
- **Global variables**: declared outside functions, visible everywhere (avoid when possible)
- Scope rules prevent name conflicts
- Global variables make code harder to test and maintain
- Use `static` for file-local globals when needed
- Prefer passing data explicitly over relying on globals

## Code Highlights

Main file and test demonstrate scope rules with user input.

**Key Lesson**: Minimize global variables. Pass data explicitly for better, testable code.