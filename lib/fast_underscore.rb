# frozen_string_literal: true

require 'fast_underscore/version'
require 'fast_underscore/fast_underscore'

if defined?(ActiveSupport)
  String.prepend(Module.new do
    def underscore
      return self unless /[A-Z-]|::/.match?(self)

      acronyms = ActiveSupport::Inflector.inflections.acronym_regex
      dup.gsub!(/(?:(?<=([A-Za-z\d]))|\b)(#{acronyms})(?=\b|[^a-z])/) do
        "#{$1 && '_'}#{$2.downcase}"
      end

      FastUnderscore.underscore(self)
    end
  end)
else
  String.prepend(Module.new do
    def underscore
      FastUnderscore.underscore(self)
    end
  end)
end
