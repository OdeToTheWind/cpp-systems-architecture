# Day 25 – Local Development Environment Setup

**Date:** April 07, 2026  
**Status:** Completed

## Goals
- Set up a professional C++ development environment
- Configure CMake, compiler, and build tools correctly
- Understand best practices for project structure
- Learn how to maintain a clean and reproducible build system

## What I Learned / Key Takeaways

- MSYS2 UCRT64 + GCC provides a reliable toolchain on Windows
- CMake + Ninja is a fast and modern build combination
- Keeping `build/` folder separate from source code is essential
- A well-structured project with clear CMake configuration scales better
- Consistent environment setup is the foundation of productive development

## Current Setup in This Repository
- Compiler: GCC via MSYS2 UCRT64
- Build System: CMake with Ninja generator
- Project Structure: Professional multi-day layout with auto-discovery
- CI/CD: GitHub Actions using MSYS2

## Key Takeaway
A clean, consistent development environment significantly reduces friction and bugs.  
This repository uses a professional CMake + Ninja + MSYS2 setup.