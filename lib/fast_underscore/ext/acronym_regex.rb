# frozen_string_literal: true

module FastUnderscore
  # Uses ActiveSupport's `acronym_regex` method to construct a memoized pattern
  # for replacing acronyms within strings that need to be underscored.
  module AcronymRegex
    def self.pattern
      return @pattern if defined?(@pattern)

      acronym_regex = ActiveSupport::Inflector.inflections.acronym_regex
      @pattern ||= /(?:(?<=([A-Za-z\d]))|\b)(#{acronym_regex})(?=\b|[^a-z])/
    end

    def underscore(string)
      return string unless /[A-Z-]|::/.match?(string)

      response = string.dup
      response.gsub!(AcronymRegex.pattern) { "#{$1 && '_'}#{$2.downcase}" }

      FastUnderscore.underscore(response)
    end
  end

  class << ActiveSupport::Inflector
    alias as_underscore underscore
    prepend AcronymRegex
  end
end
