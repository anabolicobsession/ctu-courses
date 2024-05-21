type Img = [String]

zero :: Img
zero = [
    ".##.",
    "#..#",
    "#..#",
    ".##."]

one :: Img
one = [
    "...#",
    "..##",
    "...#",
    "...#"]

bin :: Int -> String
bin 0 = "0"
bin num = reverse $ bin' num where
    bin' 0 = []
    bin' n = (if rem n 2 == 0 then '0' else '1') : bin' (quot n 2)

layer :: String -> Int -> String
layer s num = reverse ('\n' : drop 1 (reverse $ layer' s num)) where
    layer' str n = (concat $ map (\c -> (if c == '0' then zero !! n else one !! n) ++ ".") str)

main :: IO ()
main = do putStrLn "Enter integer:"
          n <- getLine
          putStrLn $ concat $ map (layer (bin (read n :: Int))) [0..3]
