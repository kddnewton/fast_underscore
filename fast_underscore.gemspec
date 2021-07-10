# frozen_string_literal: true

lib = File.expand_path('lib', __dir__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'fast_underscore/version'

Gem::Specification.new do |spec|
  spec.name          = 'fast_underscore'
  spec.version       = FastUnderscore::VERSION
  spec.authors       = ['Kevin Newton']
  spec.email         = ['kddnewton@gmail.com']

  spec.summary       = 'Fast String#underscore implementation'
  spec.description   = 'Provides a C-optimized method for underscoring a string'
  spec.homepage      = 'https://github.com/kddeisz/fast_underscore'
  spec.license       = 'MIT'

  spec.files         = `git ls-files -z`.split("\x0").reject do |f|
    f.match(%r{^(test|spec|features)/})
  end
  spec.bindir        = 'exe'
  spec.executables   = spec.files.grep(%r{^exe/}) { |f| File.basename(f) }
  spec.require_paths = ['lib']
  spec.extensions    = ['ext/fast_underscore/extconf.rb']

  spec.add_development_dependency 'benchmark-ips', '~> 2'
  spec.add_development_dependency 'bundler', '~> 2'
  spec.add_development_dependency 'minitest', '~> 5'
  spec.add_development_dependency 'rake', '~> 13'
  spec.add_development_dependency 'rake-compiler', '~> 1'
  spec.add_development_dependency 'rubocop', '~> 1.12'
  spec.add_development_dependency 'steep'
end
