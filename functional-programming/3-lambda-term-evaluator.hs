module Hw3 where
 
type Symbol = String
data Expr = Var Symbol | App Expr Expr | Lambda Symbol Expr deriving Eq
 
instance Show Expr where
    show (Var x) = x
    show (App e1 e2) = "(" ++ show e1 ++ " " ++ show e2 ++ ")"
    show (Lambda x e) = "(\\" ++ x ++ "." ++ show e ++ ")"
 
symbols :: [Symbol]
symbols = ["a" ++ show n | n <- [(0 :: Int)..]]
 
isFree :: Symbol -> Expr -> Bool
isFree x e' = isFreeHlp e' False where
    isFreeHlp (Var v) underScope = v == x && not underScope
    isFreeHlp (App e1 e2) underScope = isFreeHlp e1 underScope || isFreeHlp e2 underScope
    isFreeHlp (Lambda v e) underScope = isFreeHlp e (underScope || v == x)
 
betaReduc :: Expr -> Symbol -> Expr -> Int -> (Expr, Int)
betaReduc (Var v) x xe n | v == x = (xe, n)
                         | otherwise = (Var v, n)
betaReduc (App e1 e2) x xe n = (App e1' e2', n'') where
    (e1', n') = betaReduc e1 x xe n
    (e2', n'') = betaReduc e2 x xe n'
betaReduc lambda @ (Lambda v e) x xe n | v == x = (lambda, n)
                                       | not (isFree v xe) = let (e', n') = betaReduc e x xe n in (Lambda v e', n')
                                       | otherwise = 
                                           let fresh = symbols !! n
                                               (e', n') = betaReduc e v (Var fresh) (n + 1)
                                               (e'', n'') = betaReduc e' x xe n'
                                           in (Lambda fresh e'', n'')

hasRedex :: Expr -> Bool
hasRedex (Var _) = False
hasRedex (App (Lambda _ _) _) = True
hasRedex (App e1 e2) = hasRedex e1 || hasRedex e2
hasRedex (Lambda _ e) = hasRedex e
 
evalHlp2 :: (Expr, Int) -> (Expr, Int)
evalHlp2 (ex, i) | hasRedex ex = evalHlp2 $ evalHlp (ex, i) 
                 | otherwise = (ex, i) 
    where evalHlp (Var v, n) = (Var v, n)
          evalHlp (Lambda v e, n) = (Lambda v e', n') where (e', n') = evalHlp (e, n)
          evalHlp (App (Lambda v body) e2, n) = betaReduc body v e2 n
          evalHlp (App e1 e2, n) = (App e1' e2', n'') where
              (e1', n') = evalHlp (e1, n)
              (e2', n'') = evalHlp (e2, n')
 
eval :: Expr -> Expr
eval e = fst $ evalHlp2 (e, 0)
