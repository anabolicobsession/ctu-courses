import Data.Complex

findIdx :: Eq a => a -> [a] -> Int
findIdx el [] = error "element not found"
findIdx el list | el == head list = 0
                | otherwise = 1 + findIdx el (tail list)

tokenize :: Eq a => a -> [a] -> [[a]]
tokenize sep list | sep `elem` list = let n = findIdx sep list 
                                      in take n list : tokenize sep (drop (n + 1) list)
                  | otherwise = [list]

readRange :: String -> (Double, Double, Double)
readRange str = let tokens = tokenize ',' $ filter (`notElem` "()") str
                    parse n = read (tokens !! n) :: Double
                in (parse 0, parse 1, parse 2)

parseRange :: String -> [Double]
parseRange str = let (start, step, stop) = readRange str
                 in [start, (start + step) .. stop]

cartesian :: Functor t => (a -> a -> t a) -> [a] -> [a] -> [[t a]]
cartesian op l = map (\x -> map (`op` x) l)

pseudoMandelbrot :: Complex Double -> Int -> Complex Double
pseudoMandelbrot c n = iterate (\z -> z^2 + c) 0 !! n

isMandelbrot :: Complex Double -> Bool
isMandelbrot c = let c' = pseudoMandelbrot c 100
                 in (realPart c' ^ 2 + imagPart c' ^ 2) < 4

complexToChar :: Complex Double -> Char
complexToChar c | isMandelbrot c = 'x'
                | otherwise = '.'

printGrid :: [String] -> IO()
printGrid [] = return ()
printGrid strs = do putStrLn $ head strs
                    printGrid $ tail strs

main :: IO()
main = do putStrLn "Enter x-range (start,step,stop):"
          inp1 <- getLine
          let xRng = parseRange inp1
          putStrLn "Enter y-range (start,step,stop):"
          inp2 <- getLine
          let yRng = parseRange inp2
          let grid = map (map complexToChar) (cartesian (:+) xRng yRng)
          printGrid grid
