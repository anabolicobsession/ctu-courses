module Hw4 where

import Control.Applicative
import Data.Char
import Parser
import Hw3

type Def = (Symbol, Expr)

space :: Parser ()
space = many (sat isSpace) *> pure ()

var :: Parser Expr
var = do an <- some alphaNum
         return (Var an)

lambda :: Parser Expr
lambda = do char '('
            string "\\"
            v <- some alphaNum
            char '.'
            e <- expr
            char ')'
            return (Lambda v e)

app :: Parser Expr
app = do char '('
         e1 <- expr
         sep
         e2 <- expr
         char ')'
         return (App e1 e2)

expr :: Parser Expr
expr = var <|> lambda <|> app

def :: Parser Def
def = do v <- some alphaNum
         sep
         string ":="
         sep
         e <- expr
         return (v,e)


subsWithDef :: Expr -> Def -> Expr
subsWithDef e' d' = subsWithExpr e' (fst d') (snd d') where
    subsWithExpr :: Expr -> Symbol -> Expr -> Expr
    subsWithExpr (Var v) s ex | v == s = ex
                              | otherwise = (Var v)
    subsWithExpr (Lambda v e) s ex = (Lambda v (subsWithExpr e s ex))
    subsWithExpr (App e1 e2) s ex = (App (subsWithExpr e1 s ex) (subsWithExpr e2 s ex))

substituteDefs :: [Def] -> [Def]
substituteDefs = subsDed [] where
    subsDed :: [Def] -> [Def] -> [Def]
    subsDed ds []       = ds
    subsDed [] (sd:sds) = subsDed [sd] sds
    subsDed ds (sd:sds) = subsDed (addDef ds sd) sds where
        addDef :: [Def] -> Def -> [Def]
        addDef ds sd = foldl subsDed sd ds : ds where
            subsDed :: Def -> Def -> Def
            subsDed d d' = (fst d, subsWithDef (snd d) d')

subsWithDefs :: Expr -> [Def] -> Expr
subsWithDefs e ds = foldl subsWithDef e ds

defs :: Parser [Def]
defs = do ds <- many (def <* sep)
          return $ substituteDefs ds

prg :: Parser Expr
prg = do ds <- defs
         e <- expr
         space
         return $ subsWithDefs e ds

readPrg :: String -> Maybe Expr
readPrg inp = case parse prg inp of
    Just (e,"") -> return e
    _ -> Nothing
