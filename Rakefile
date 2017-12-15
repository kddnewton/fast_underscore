require 'bundler/gem_tasks'
require 'rake/testtask'
require 'rake/extensiontask'

Rake::ExtensionTask.new(:compile) do |ext|
  ext.name = 'fast_underscore'
  ext.ext_dir = 'ext/fast_underscore'
  ext.lib_dir = 'lib/fast_underscore'
  ext.gem_spec = Gem::Specification.load('fast_underscore.gemspec')
end

Rake::TestTask.new(:test) do |t|
  t.libs << 'test'
  t.libs << 'lib'
  t.test_files = FileList['test/**/*_test.rb']
end

Rake::Task[:test].prerequisites << :compile

task default: :test
