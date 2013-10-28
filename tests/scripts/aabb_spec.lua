require 'dabes.aabb'

describe("AABB", function()
    describe("type and construction", function()
        it("should properly construct the bounding box from xywh", function()
            local box = AABB.new(1, 2, 3, 4)
            assert.are_equal(box.x,        1)
            assert.are_equal(box.origin.x, 1)
            assert.are_equal(box[1],       1)

            assert.are_equal(box.y,        2)
            assert.are_equal(box.origin.y, 2)
            assert.are_equal(box[2],       2)

            assert.are_equal(box.w,        3)
            assert.are_equal(box.size.x,   3)
            assert.are_equal(box[3],       3)

            assert.are_equal(box.h,        4)
            assert.are_equal(box.size.y,   4)
            assert.are_equal(box[4],       4)
        end)

        it("should properly construct the bounding box from origin and size", function()
            local box = AABB.new(VPoint.new(1, 2), VPoint.new(3, 4))
            assert.are_equal(box.x,        1)
            assert.are_equal(box.origin.x, 1)
            assert.are_equal(box[1],       1)

            assert.are_equal(box.y,        2)
            assert.are_equal(box.origin.y, 2)
            assert.are_equal(box[2],       2)

            assert.are_equal(box.w,        3)
            assert.are_equal(box.size.x,   3)
            assert.are_equal(box[3],       3)

            assert.are_equal(box.h,        4)
            assert.are_equal(box.size.y,   4)
            assert.are_equal(box[4],       4)
        end)

        it("should know an AABB when it sees one", function()
            local box = AABB.new(1, 2, 3, 4)
            assert.is_true(isaabb(box))
        end)
    end)

    describe("metamethods", function()
        local box, adj
        setup(function()
            box = AABB.new(1, 2, 3, 4)
            adj = VPoint.new(1, 1)
        end)

        it("should be able to adjust the origin with the subtraction operator", function()
            local newbox = box - adj
            assert.are_equal(newbox.x, 0)
            assert.are_equal(newbox.y, 1)
            assert.are_equal(newbox.w, 3)
            assert.are_equal(newbox.h, 4)
            assert.are_equal(box.x, 1)
            assert.are_equal(box.y, 2)
        end)

        it("should be able to adjust the origin with the addition operator", function()
            local newbox = box + adj
            assert.are_equal(newbox.x, 2)
            assert.are_equal(newbox.y, 3)
            assert.are_equal(newbox.w, 3)
            assert.are_equal(newbox.h, 4)
            assert.are_equal(box.x, 1)
            assert.are_equal(box.y, 2)
        end)
    end)
end)
