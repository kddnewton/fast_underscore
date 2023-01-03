# frozen_string_literal: true

require "bundler/gem_tasks"
require "rake/testtask"
require "rake/extensiontask"
require "ruby_memcheck"
require "syntax_tree/rake_tasks"

RubyMemcheck.config(binary_name: "fast_underscore")

Rake::ExtensionTask.new(:compile) do |ext|
  ext.name = "fast_underscore"
  ext.ext_dir = "ext/fast_underscore"
  ext.lib_dir = "lib/fast_underscore"
  ext.gem_spec = Gem::Specification.load("fast_underscore.gemspec")
end

config =
  lambda do |t|
    t.libs << "test"
    t.test_files = FileList["test/**/*_test.rb"]
  end

Rake::TestTask.new(test: :compile, &config)

namespace :test do
  RubyMemcheck::TestTask.new(valgrind: :compile, &config)
end

task default: :test

configure = ->(task) do
  task.source_files =
    FileList[%w[Gemfile Rakefile *.gemspec lib/**/*.rb test/**/*.rb]]
end

SyntaxTree::Rake::CheckTask.new(&configure)
SyntaxTree::Rake::WriteTask.new(&configure)
