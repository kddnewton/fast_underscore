# frozen_string_literal: true

require 'fast_underscore/version'
require 'fast_underscore/fast_underscore'

module FastUnderscore
  # Hooks into ActiveSupport::Inflector and waits for the #underscore method to
  # be defined. When it is, it automatically redefines it.
  module ActiveSupportedDelayedPatch
    def method_added(method)
      FastUnderscore.active_support if method == :underscore
      super
    end
  end

  # Depending on `ActiveSupport::VERSION`, `::install` determines the manner in
  # which acronyms are handled, then it redefines the
  # `ActiveSupport::Inflector::underscore` method to use the `FastUnderscore`
  # native extension. It leaves the existing `ActiveSupport` monkeypatch on
  # `String` that allows it to call into the newly redefined `Inflector` method.
  def self.active_support
    require 'active_support/version'
    gem_version = Gem::Version.new(ActiveSupport::VERSION::STRING)

    if gem_version >= Gem::Version.new('5.2.0')
      require_relative 'fast_underscore/acronym_underscore_regex'
    else
      require_relative 'fast_underscore/acronym_regex'
    end
  end
end

if defined?(ActiveSupport)
  FastUnderscore.active_support
else
  module ActiveSupport
    module Inflector
      singleton_class.prepend(::FastUnderscore::ActiveSupportedDelayedPatch)
    end
  end
end
