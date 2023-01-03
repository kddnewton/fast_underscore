# frozen_string_literal: true

require "fast_underscore/version"
require "fast_underscore/fast_underscore"

module FastUnderscore
  # Override ActiveSupport::Inflector::underscore to use
  # FastUnderscore::underscore for ActiveSupport < 5.2.0.
  module ActiveSupportInflectorOldPatch
    def self.pattern
      return @pattern if defined?(@pattern)

      acronym_regex = ActiveSupport::Inflector.inflections.acronym_regex
      @pattern = /(?:(?<=([A-Za-z\d]))|\b)(#{acronym_regex})(?=\b|[^a-z])/
    end

    def underscore(string)
      return string unless /[A-Z-]|::/.match?(string)

      response = string.dup
      response.gsub!(ActiveSupportInflectorOldPatch.pattern) do
        "#{$1 && "_"}#{$2.downcase}"
      end

      FastUnderscore.underscore(response)
    end
  end

  # Override ActiveSupport::Inflector::underscore to use
  # FastUnderscore::underscore for ActiveSupport >= 5.2.0.
  module ActiveSupportInflectorPatch
    def underscore(string)
      return string unless /[A-Z-]|::/.match?(string)

      response = string.dup
      acronyms = ActiveSupport::Inflector.inflections.acronyms_underscore_regex

      response.gsub!(acronyms) { "#{$1 && "_"}#{$2.downcase}" }

      FastUnderscore.underscore(response)
    end
  end

  # Hooks into ActiveSupport::Inflector and waits for the #underscore method to
  # be defined. When it is, it automatically redefines it.
  module ActiveSupportedDelayedPatch
    def method_added(method)
      FastUnderscore.active_support if method == :underscore
      super
    end
  end

  # Override the String#underscore method no matter when it was defined so that
  # we"re sure it"s going to call the correct implementation.
  module ActiveSupportStringPatch
    def underscore
      ActiveSupport::Inflector.underscore(self)
    end
  end

  # Depending on ActiveSupport::VERSION, ::active_support determines the manner
  # in which acronyms are handled, then it redefines the
  # ActiveSupport::Inflector::underscore method to use the FastUnderscore
  # native extension.
  def self.active_support
    require "active_support/version"
    gem_version = Gem::Version.new(ActiveSupport::VERSION::STRING)

    if gem_version >= Gem::Version.new("5.2.0")
      ActiveSupport::Inflector.prepend(ActiveSupportInflectorPatch)
    else
      ActiveSupport::Inflector.prepend(ActiveSupportInflectorOldPatch)
    end

    ActiveSupport::Inflector.alias_method(:as_underscore, :underscore)
    String.prepend(ActiveSupportStringPatch)
  end
end

if defined?(ActiveSupport)
  FastUnderscore.active_support
else
  module ActiveSupport
    module Inflector
      prepend(FastUnderscore::ActiveSupportedDelayedPatch)
    end
  end
end
