require 'dabes.vector'

describe("VPoint", function()
    describe("type and construction", function()
        it("should be able to construct with numbers", function()
            local vp = VPoint.new(1, 2)
            assert.are_equal(vp.x, 1)
            assert.are_equal(vp.y, 2)
            assert.are_equal(vp[1], 1)
            assert.are_equal(vp[2], 2)
        end)

        it("should be able to construct with a table", function()
            local vp = VPoint.new({1, 2})
            assert.are_equal(vp.x, 1)
            assert.are_equal(vp.y, 2)
            assert.are_equal(vp[1], 1)
            assert.are_equal(vp[2], 2)
        end)

        it("should pass the isvpoint check", function()
            local vp = VPoint.new(1, 2)
            assert.is_true(isvpoint(vp))
        end)
    end)

    describe("metamethods", function()
        local a, b
        before_each(function()
            a = VPoint.new(1, 2)
            b = VPoint.new(3, 4)
        end)

        it("should invert x and y with the unary minus operator", function()
            local c = -a
            assert.are_equal(c.x, -1)
            assert.are_equal(c.y, -2)
        end)

        it("should be able to add two VPoints with the addition operator", function()
            local c = a + b
            assert.are_equal(c.x, 4)
            assert.are_equal(c.y, 6)
        end)

        it("should be able to subtract two VPoints with the subtraction operator", function()
            local c = a - b
            assert.are_equal(c.x, -2)
            assert.are_equal(c.y, -2)
        end)

        it("should be able to multiply two VPoints with the multiplication operator", function()
            local c = a * b
            assert.are_equal(c.x, 3)
            assert.are_equal(c.y, 8)
        end)

        it("should be able to scale by a number with the multiplication operator", function()
            local c = a * 10
            assert.are_equal(c.x, 10)
            assert.are_equal(c.y, 20)
        end)

        it("should be able to scale by a number with the division operator", function()
            local c = a / 2
            assert.are_equal(c.x, 0.5)
            assert.are_equal(c.y, 1)
        end)
    end)

    describe("methods", function()
        local a
        before_each(function()
            a = VPoint.new(3, 4)
        end)

        it("should be able to find the magnitude with mag()", function()
            local mag = a:mag()
            assert.are_equal(mag, 5)
        end)
    end)
end)
