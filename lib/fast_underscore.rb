# frozen_string_literal: true

require 'fast_underscore/version'
require 'fast_underscore/fast_underscore'

module FastUnderscore
  # Hooks into ActiveSupport::Inflector and waits for the #underscore method to
  # be defined. When it is, it automatically redefines it by hooking into its
  # acronyms and then still calling the native extension.
  module ActiveSupportHook
    def method_added(method)
      return super if method != :underscore || !defined?(ActiveSupport)
      require 'active_support/version'

      if ActiveSupport::VERSION::MAJOR == 5 &&
         ActiveSupport::VERSION::MINOR >= 2
        require 'fast_underscore/ext/acronym_underscore_regex'
      else
        require 'fast_underscore/ext/acronym_regex'
      end
    end
  end
end

module ActiveSupport
  module Inflector
    class << self
      prepend FastUnderscore::ActiveSupportHook
    end
  end
end
