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

        it("should derive the corners", function()
            local box = AABB.new(1, 1, 9, 9)

            assert.are_equal(box.tl.x, 1)
            assert.are_equal(box.tl.y, 1)

            assert.are_equal(box.tr.x, 10)
            assert.are_equal(box.tr.y, 1)

            assert.are_equal(box.bl.x, 1)
            assert.are_equal(box.bl.y, 10)

            assert.are_equal(box.br.x, 10)
            assert.are_equal(box.br.y, 10)
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

        it("should be able to test equivalence", function()
            local a = AABB.new(1, 1, 1, 1)
            local b = AABB.new(1, 1, 1, 1)
            local c = AABB.new(1, 1, 1, 2)
            local d = 1
            assert.is_true(a == b)
            assert.is_false(a == c)
            assert.is_false(a == d)
        end)
    end)

    describe("methods", function()
        local bigbox
        local inpoint, outpoint
        local inbox, midbox, outbox

        setup(function()
            bigbox = AABB.new(0, 0, 10, 10)
            inpoint = VPoint.new(5, 5)
            outpoint = VPoint.new(11, 11)
            inbox = AABB.new(2, 2, 2, 2)
            midbox = AABB.new(2, 2, 10, 10)
            outbox = AABB.new(11, 11, 2, 2)
        end)

        it("should be able to test containment of VPoints", function()
            assert.is_true(bigbox:contains(inpoint))
            assert.is_false(bigbox:contains(outpoint))
        end)

        it("should be able to test containment of AABBs", function()
            assert.is_true(bigbox:contains(inbox))
            assert.is_false(bigbox:contains(midbox))
            assert.is_false(bigbox:contains(outbox))
        end)

        it("should be able to test intersection of AABBs", function()
            assert.is_true(bigbox:intersects(inbox))
            assert.is_true(bigbox:intersects(midbox))
            assert.is_false(bigbox:intersects(outbox))

            -- Same tests the other way around
            assert.is_true(inbox:intersects(bigbox))
            assert.is_true(midbox:intersects(bigbox))
            assert.is_false(outbox:intersects(bigbox))
        end)

        it("should be able to convert a point to its coordinate space", function()
            local converted = outbox:convert(outpoint)
            assert.same(converted, VPoint.new(0, 0))
        end)

        it("should be able to convert a point from its coordinate space", function()
            local c = VPoint.new(0, 0)
            local cfrom = outbox:convert_from(c)
            assert.same(cfrom, outpoint)
        end)

        it("should be able to convert an AABB to its coordinate space", function()
            local converted = outbox:convert(outbox)
            local should = AABB.new(VPoint.new(0, 0), outbox.size)
            assert.same(converted, should)
        end)

        it("should be able to convert an AABB from its coordinate space", function()
            local c = AABB.new(VPoint.new(0, 0), outbox.size)
            local cfrom = outbox:convert_from(c)
            assert.same(cfrom, outbox)
        end)
    end)
end)
