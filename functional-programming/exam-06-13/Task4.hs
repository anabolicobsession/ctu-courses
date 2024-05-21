module Task4 ( propagateUnits, Literal (..) ) where
import Data.List -- for delete, nub functions

type Variable = String
data Literal = Neg { variable :: Variable } | Pos { variable :: Variable } deriving (Eq, Ord)
type Clause = [Literal]
type Formula = [Clause]

instance Show Literal where
    show (Neg x) = "-" ++ x
    show (Pos x) = x

formula1 :: Formula
formula1 = [[Neg "x"]]

formula2 :: Formula
formula2 = [[Pos "x"], [Neg "x"], [Pos "y"], [Neg "y"]]

formula3:: Formula
formula3 = [[Pos "a", Pos "b", Neg "c", Neg "f"], [Pos "b", Pos "c"], [Neg "b", Pos "e"], [Neg "b"]]

neg :: Literal -> Literal
neg (Pos v) = Neg v
neg (Neg v) = Pos v

hasUnit :: Formula -> Bool
hasUnit = foldl (\res c -> res || length c == 1) False

popUnit :: Formula -> (Formula, Literal)
popUnit f = let unit = head $ head $ filter (\c -> length c == 1) f
                f' = filter (\c -> c /= [unit]) f
            in (f', unit)

propagateUnit :: Literal -> Formula -> Formula
propagateUnit _ [] = []
propagateUnit u (c:cs) | u `elem` c = propagateUnit u cs
                       | neg u `elem` c = delete (neg u) c : propagateUnit u cs
                       | otherwise = c : propagateUnit u cs

propagateUnits :: Formula -> Formula
propagateUnits f | hasUnit f = let (f', unit) = popUnit f
                               in propagateUnits $ propagateUnit unit f'
                 | otherwise = nub f
