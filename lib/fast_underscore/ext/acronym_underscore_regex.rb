# frozen_string_literal: true

module FastUnderscore
  # Uses ActiveSupport's `acronym_underscore_regex` method for replacing
  # acronyms within strings that need to be underscored.
  module AcronymUnderscoreRegex
    def underscore(string)
      return string unless /[A-Z-]|::/.match?(string)

      response = string.dup
      acronyms = ActiveSupport::Inflector.inflections.acronyms_underscore_regex

      response.gsub!(acronyms) { "#{$1 && '_'}#{$2.downcase}" }

      FastUnderscore.underscore(response)
    end
  end

  class << ActiveSupport::Inflector
    alias as_underscore underscore
    prepend AcronymUnderscoreRegex
  end
end
