# frozen_string_literal: true

require 'fast_underscore/version'
require 'fast_underscore/fast_underscore'

if !defined?(ActiveSupport)
  require 'fast_underscore/ext/plain_string'
elsif ActiveSupport::VERSION::MAJOR == 5 && ActiveSupport::VERSION::MINOR >= 2
  require 'fast_underscore/ext/acronym_underscore_regex'
else
  require 'fast_underscore/ext/acronym_regex'
end
