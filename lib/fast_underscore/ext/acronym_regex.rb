# frozen_string_literal: true

module FastUnderscore
  # Uses ActiveSupport's `acronym_regex` method to construct a memoized pattern
  # for replacing acronyms within strings that need to be underscored.
  module AcronymRegex
    class << self
      private

      def pattern
        return @pattern if defined?(@pattern)

        acronym_regex = ActiveSupport::Inflector.inflections.acronym_regex
        @pattern ||= /(?:(?<=([A-Za-z\d]))|\b)(#{acronym_regex})(?=\b|[^a-z])/
      end
    end

    def underscore
      return self unless /[A-Z-]|::/.match?(self)

      response = dup
      response.gsub!(AcronymRegex.pattern) { "#{$1 && '_'}#{$2.downcase}" }

      FastUnderscore.underscore(response)
    end
  end
end

String.prepend(FastUnderscore::AcronymRegex)

class << ActiveSupport::Inflector
  define_method(:underscore, &FastUnderscore.method(:underscore))
end
