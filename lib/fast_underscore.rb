# frozen_string_literal: true

require 'fast_underscore/version'
require 'fast_underscore/fast_underscore'

module FastUnderscore
  # Depending on `ActiveSupport::VERSION`, `::install` determines the manner in
  # which acronyms are handled, then it redefines the
  # `ActiveSupport::Inflector::underscore` method to use the `FastUnderscore`
  # native extension. It leaves the existing `ActiveSupport` monkeypatch on
  # `String` that allows it to call into the newly redefined `Inflector` method.
  def self.install
    require 'active_support/version'

    if ActiveSupport::VERSION::MAJOR == 5 && ActiveSupport::VERSION::MINOR >= 2
      require 'fast_underscore/acronym_underscore_regex'
    else
      require 'fast_underscore/acronym_regex'
    end
  end
end

if defined?(ActiveSupport)
  FastUnderscore.install
else
  module ActiveSupport
    module Inflector
      class << self
        prepend(
          Module.new do
            # Hooks into ActiveSupport::Inflector and waits for the #underscore
            # method to be defined. When it is, it automatically redefines it.
            # Using this `prepend` trick to attempt to be a good citizen in the
            # case that someone else has already hooked into `method_added` on
            # `Inflector`.
            def method_added(method)
              method == :underscore ? FastUnderscore.install : super
            end
          end
        )
      end
    end
  end
end
