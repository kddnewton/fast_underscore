# frozen_string_literal: true

module FastUnderscore
  # Extends String to include an underscore method that delegates over to
  # FastUnderscore's `#underscore` method.
  module StringExtension
    def underscore
      FastUnderscore.underscore(self)
    end
  end
end

String.prepend(FastUnderscore::StringExtension)
