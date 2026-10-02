# Changelog

All notable changes to logslice are documented here. The format follows Keep a Changelog and the
project uses Semantic Versioning.

## [Unreleased]

## [1.3.0] - 2026-06-16

### Added
- `--count` prints the number of matching lines instead of the lines.

### Fixed
- Lines without a timestamp no longer end a slice early.

## [1.2.1] - 2026-05-02

### Fixed
- `--until` is inclusive, as documented.

## [1.2.0] - 2026-04-10

### Added
- `--level` filters by log level.

## [1.0.0] - 2026-02-01

### Added
- First release: `--from` and `--until` time ranges.
