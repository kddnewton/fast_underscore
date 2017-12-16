require "test_helper"

class FastUnderscoreTest < Minitest::Test
  def test_underscore
    assert_equal 'foo/bar_baz', 'foo::bar-baz'.underscore
    assert_equal 'foo_bar_baz', 'FooBarBaz'.underscore
    assert_equal 'foo123_bar', 'Foo123Bar'.underscore
  end

  def test_fuzzing
    source = %w[_ - : :: / 漢字 😊🎉] +
             ('a'..'z').to_a + ('A'..'Z').to_a + ('0'..'9').to_a

    500.times do
      word = Array.new(100) { source.sample }.join

      expected = active_support_underscore(word)
      actual = word.underscore

      assert_equal expected, actual, "Word: #{word}"
    end
  end

  private

  def active_support_underscore(camel_cased_word)
    return camel_cased_word unless /[A-Z-]|::/.match?(camel_cased_word)
    word = camel_cased_word.to_s.gsub('::'.freeze, '/'.freeze)
    word.gsub!(/([A-Z\d]+)([A-Z][a-z])/, '\1_\2'.freeze)
    word.gsub!(/([a-z\d])([A-Z])/, '\1_\2'.freeze)
    word.tr!('-'.freeze, '_'.freeze)
    word.downcase!
    word
  end
end
