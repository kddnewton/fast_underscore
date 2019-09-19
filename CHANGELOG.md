# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/) and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.2.0] - 2018-01-02

### Changed

- Instead of needing `ActiveSupport` to be loaded before requiring `fast_underscore`, `fast_underscore` can now detect when `ActiveSupport` redefines the `underscore` method and automatically redefine at that point. Note that this basically flips the requirement such that `fast_underscore` needs to be loaded first (which is actually much faster).

## [0.1.0] - 2017-12-17

### Added

- Support for the Rails 5.2 beta.

[unreleased]: https://github.com/kddeisz/fast_underscore/compare/v0.2.0...HEAD
[0.2.0]: https://github.com/kddeisz/fast_underscore/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/kddeisz/fast_underscore/compare/6981d0...v0.1.0
