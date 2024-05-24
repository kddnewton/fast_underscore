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

  spec.files =
    `git ls-files -z`.split("\x0")
      .reject { |f| f.match(%r{^(test|spec|features|Gemfile.lock)/}) }

  spec.bindir = "exe"
  spec.executables = spec.files.grep(%r{^exe/}) { |f| File.basename(f) }
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
