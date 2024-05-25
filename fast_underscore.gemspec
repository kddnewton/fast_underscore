# frozen_string_literal: true

require_relative "lib/fast_underscore/version"

version = FastUnderscore::VERSION
repository = "https://github.com/kddnewton/fast_underscore"

Gem::Specification.new do |spec|
  spec.name = "fast_underscore"
  spec.version = version
  spec.authors = ["Kevin Newton"]
  spec.email = ["kddnewton@gmail.com"]

  spec.summary = "Fast String#underscore implementation"
  spec.description = "Provides a C-optimized method for underscoring a string"
  spec.homepage = repository
  spec.license = "MIT"

  spec.metadata = {
    "bug_tracker_uri" => "#{repository}/issues",
    "changelog_uri" => "#{repository}/blob/v#{version}/CHANGELOG.md",
    "source_code_uri" => repository,
    "rubygems_mfa_required" => "true"
  }

  spec.files = [
    "CHANGELOG.md",
    "CODE_OF_CONDUCT.md",
    "LICENSE",
    "README.md",
    "ext/fast_underscore/extconf.rb",
    "ext/fast_underscore/fast_underscore.c",
    "ext/fast_underscore/fast_underscore.h",
    "fast_underscore.gemspec",
    "lib/fast_underscore.rb",
    "lib/fast_underscore/version.rb"
  ]

  spec.require_paths = ["lib"]
  spec.extensions = ["ext/fast_underscore/extconf.rb"]

  spec.add_development_dependency "benchmark-ips", "~> 2"
  spec.add_development_dependency "bundler", "~> 2"
  spec.add_development_dependency "minitest", "~> 5"
  spec.add_development_dependency "rake", "~> 13"
  spec.add_development_dependency "rake-compiler", "~> 1"
  spec.add_development_dependency "rubocop", "~> 1.12"
  spec.add_development_dependency "ruby_memcheck"
  spec.add_development_dependency "syntax_tree"
end
