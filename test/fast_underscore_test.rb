require "test_helper"

class FastUnderscoreTest < Minitest::Test
  def test_underscore
    assert_equal 'foo', 'foo'.underscore
  end
end
