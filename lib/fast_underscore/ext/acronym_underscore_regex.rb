# frozen_string_literal: true

module FastUnderscore
  # Uses ActiveSupport's `acronym_underscore_regex` method for replacing
  # acronyms within strings that need to be underscored.
  module AcronymUnderscoreRegex
    def underscore
      return self unless /[A-Z-]|::/.match?(self)

      response = dup
      acronyms = ActiveSupport::Inflector.inflections.acronyms_underscore_regex

      response.gsub!(acronyms) { "#{$1 && '_'}#{$2.downcase}" }

      FastUnderscore.underscore(response)
    end
  end
end

String.prepend(FastUnderscore::AcronymUnderscoreRegex)

class << ActiveSupport::Inflector
  define_method(:underscore, &FastUnderscore.method(:underscore))
end
