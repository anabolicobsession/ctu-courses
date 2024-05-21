module Task4 (Tree (..), buildHeap) where

data Tree a = Leaf | Node a (Tree a) (Tree a)

tostr :: (Show a) => Tree a -> Int -> String
tostr Leaf d = ""
tostr (Node x l r) d = tostr l (d+1) ++ concat (replicate d "---") ++ show x ++ "\n" ++ tostr r (d+1)

instance (Show a) => Show (Tree a) where
    show tree = tostr tree 0

minDepth :: Tree a -> Int
minDepth Leaf = 0
minDepth (Node _ l r) = 1 + min (minDepth l) (minDepth r)

insert :: a -> Tree a -> Tree a
insert x Leaf = Node x Leaf Leaf
insert x (Node v l r) | minDepth l <= minDepth r = Node v (insert x l) r
                      | otherwise = Node v l (insert x r)

tryLeftSwap :: (Eq a, Ord a) => Tree a -> Tree a
tryLeftSwap t @ (Node v (Node lv ll lr) r) | v < lv = Node lv (Node v ll lr) r
                                           | otherwise = t
tryLeftSwap t = t

tryRightSwap :: (Eq a, Ord a) => Tree a -> Tree a
tryRightSwap t @ (Node v l (Node rv rl rr)) | v < rv = Node rv l (Node v rl rr)
                                            | otherwise = t
tryRightSwap t = t

trySwap :: (Eq a, Ord a) => Tree a -> Tree a
trySwap = tryLeftSwap . tryRightSwap

enforce :: (Eq a, Ord a) => Tree a -> Tree a
enforce Leaf = Leaf
enforce (Node v l r) = trySwap (Node v (enforce l) (enforce r))

buildHeap :: (Eq a, Ord a) => [a] -> Tree a
buildHeap = foldl (\t x -> enforce $ insert x t) Leaf