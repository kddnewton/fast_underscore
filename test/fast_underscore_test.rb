require "test_helper"

class FastUnderscoreTest < Minitest::Test
  def test_underscore
    assert_equal 'foo/bar_baz', 'foo::bar-baz'.underscore
    assert_equal 'foo_Bar_Baz', 'fooBarBaz'.underscore
  end
end
