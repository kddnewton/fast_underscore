# frozen_string_literal: true

require "bundler/gem_tasks"
require "rake/testtask"
require "rake/extensiontask"
require "ruby_memcheck"

RubyMemcheck.config(
  binary_name: "fast_underscore",
  skipped_ruby_functions: [
    *RubyMemcheck::Configuration::DEFAULT_SKIPPED_RUBY_FUNCTIONS,
    # Explicitly ignoring this function as it allocates a string to the heap but
    # Ruby doesn't free it. This is the final function that we call to return
    # the underscored string back to the user.
    /\Arb_enc_str_new\z/
  ]
)

Rake::ExtensionTask.new(:compile) do |ext|
  ext.name = "fast_underscore"
  ext.ext_dir = "ext/fast_underscore"
  ext.lib_dir = "lib/fast_underscore"
  ext.gem_spec = Gem::Specification.load("fast_underscore.gemspec")
end

config = lambda do |t|
  t.libs << "test"
  t.test_files = FileList["test/**/*_test.rb"]
end

Rake::TestTask.new(test: :compile, &config)

namespace :test do
  RubyMemcheck::TestTask.new(valgrind: :compile, &config)
end

task default: :test
