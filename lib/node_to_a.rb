class Node

  class ARRAY < Node
    def to_a
      a = []
      e = self
      while e do
        a << e.head
        e = e.next
      end
      a
    end
  end

  class ZARRAY < Node
    def to_a
      []
    end
  end

  class BLOCK < Node
    def to_a
      a = []
      e = self
      while e do
        a << e.head
        e = e.next
      end
      a
    end
  end

end

