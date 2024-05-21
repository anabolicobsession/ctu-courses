import Data.List (transpose)
import Control.Monad

type Vector = [Float]
type Matrix = [[Float]]

matrix1 :: Matrix
matrix1 = [[2, 0], [0, 0.5], [1, 1]]

matrix2 :: Matrix
matrix2 = [[1, 0], [0, 1]]

scalarProd :: Vector -> Vector -> Float
scalarProd v1 v2 = sum $ zipWith (*) v1 v2

rollout :: Matrix -> Vector
rollout = concat

canBeMul :: Matrix -> Matrix -> Bool
canBeMul m1 m2 = let width2 = length (rollout m2) `div` length (head m2)
                     height1 = length $ head m1
                 in width2 == height1

mul :: Matrix -> Matrix -> Matrix
mul m1 m2 = let m2t = transpose m2
            in map (\v1 -> map (scalarProd v1) m2t) m1

splitBy :: Int -> [a] -> [[a]]
splitBy _ [] = []
splitBy n list | n > 0 = take n list : splitBy n (drop n list)
               | otherwise = [list]

parseMatrix :: String -> Matrix
parseMatrix str = let tokens = words str
                      (height, columns) = (read (head tokens) :: Int, map (\t -> read t :: Float) (tail tokens))
                  in splitBy height columns

showMatrix :: Matrix -> String
showMatrix mtx = let height = length $ head mtx
                     vec = rollout mtx
                 in show height ++ " " ++ unwords (map show vec)

main :: IO ()
main = do putStrLn "Enter A:"
          a <- fmap parseMatrix getLine
          putStrLn "Enter B:"
          b <- fmap parseMatrix getLine
          let result = if canBeMul a b then showMatrix $ mul a b else "ERROR"
          putStrLn $ "Result: " ++ result