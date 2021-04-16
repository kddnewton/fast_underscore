# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/) and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.3.2] - 2021-04-16

### Changed

- Ensure that `String#underscore` still uses acronyms when `ActiveSupport` is being loaded.

## [0.3.1] - 2018-08-18

### Changed

- Don't require `'active_support'`, just allow the user to side-load it.

## [0.3.0] - 2018-01-03

### Changed

- Use `include` instead of `prepend` for extending the inflector.

## [0.2.0] - 2018-01-02

### Changed

- Instead of needing `ActiveSupport` to be loaded before requiring `fast_underscore`, `fast_underscore` can now detect when `ActiveSupport` redefines the `underscore` method and automatically redefine at that point. Note that this basically flips the requirement such that `fast_underscore` needs to be loaded first (which is actually much faster).

## [0.1.0] - 2017-12-17

### Added

- Support for the Rails 5.2 beta.

[unreleased]: https://github.com/kddeisz/fast_underscore/compare/v0.3.2...HEAD
[0.3.2]: https://github.com/kddeisz/fast_underscore/compare/v0.3.1...v0.3.2
[0.3.1]: https://github.com/kddeisz/fast_underscore/compare/v0.3.0...v0.3.1
[0.3.0]: https://github.com/kddeisz/fast_underscore/compare/v0.2.0...v0.3.0
[0.2.0]: https://github.com/kddeisz/fast_underscore/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/kddeisz/fast_underscore/compare/6981d0...v0.1.0
