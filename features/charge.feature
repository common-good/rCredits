Feature: charge customer
  When a customer presses the 'Charge' button, the user should see a screen showing 'successful' and contatining the text: 'You charged $customer $amount for goods and services. Your reward is $amount*0.10.'
  
  Scenario: press charge button should give the amount changed and the reward amount 
    Given the charge button has been pressed
    And the entered amount is a number greater then $amount
    When I enter this $price
    Then I should get the customers $name the $amount changed and their $reward

    Examples:
      | price | name        | amount| reward | 
      |  12   |Susan Shopper|  12   |  1.20  |
      |  99   |Susan Shopper|  99   |  9.90  | 
  
