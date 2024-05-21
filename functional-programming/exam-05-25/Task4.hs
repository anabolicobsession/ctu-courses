module Task4 (minSpanningTree, Graph (..), Edge (..)) where
import Data.List -- for sortOn

data Edge a b = Edge { u :: a, v :: a, weight :: b } deriving (Eq,Show)

data Graph a b = Graph { nodes :: [a], edges :: [Edge a b] } deriving Show

gr :: Graph Char Int
gr = Graph{ nodes = ['A'..'F'],
            edges = [
                Edge 'A' 'B' 1,
                Edge 'D' 'E' 4,
                Edge 'E' 'F' 7,
                Edge 'A' 'D' 5,
                Edge 'B' 'E' 2,
                Edge 'C' 'F' 5,
                Edge 'D' 'B' 6,
                Edge 'E' 'C' 4,
                Edge 'A' 'E' 3] }

minWeight :: (Eq a, Ord b) => Graph a b -> [Edge a b] -> [a] -> Edge a b
minWeight g es covered = head $ sortOn weight filtered where
    notCovered = filter (`notElem` covered) (nodes g)
    isGoodEdge = \(Edge n1 n2 _) -> (n1 `elem` covered && n2 `elem` notCovered) || (n2 `elem` covered && n1 `elem` notCovered)
    filtered = filter isGoodEdge $ filter (`notElem` es) (edges g)

sameSets :: Eq a => [a] -> [a] -> Bool
sameSets l1 l2 | length l1 /= length l2 = False
               | otherwise = all (`elem` l2) l1

iter :: (Eq a, Ord b) => Graph a b -> [Edge a b] -> [a] -> [Edge a b]
iter g es covered = if sameSets covered (nodes g)
                    then es 
                    else let e @ (Edge n1 n2 w) = minWeight g es covered
                         in iter g (e:es) ((if n1 `elem` covered then n2 else n1) : covered)

minSpanningTree :: (Eq a, Ord b) => Graph a b -> [Edge a b]
minSpanningTree g = iter g [] [head (nodes g)]
