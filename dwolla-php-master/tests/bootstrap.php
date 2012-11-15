<?php

require_once '../lib/dwolla.php';

/**
 * Wraps var_dump. Tons less typing this way.
 * 
 * @param mixed $expression 
 */
function d($expression)
{
    var_dump($expression);
}

/**
 * Adds a die() statment to d()
 * 
 * @param mixed $expression 
 */
function dd($expression)
{
    d($expression);
    die();
}