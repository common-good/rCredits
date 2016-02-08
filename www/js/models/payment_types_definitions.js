(function(window) {

  window.paymentTypesDefinitions = [
    {
      name: 'Cash',
      fee: {
        title: 'zero',
        value: 0,
        unit: 'cash'
      }
    },
    {
      name: 'Credit card',
      fee: {
        title: '3%',
        value: 3,
        unit: 'percent'
      }
    },
    {
      name: 'Check',
      fee: {
        title: '$3',
        value: 3,
        unit: 'cash'
      }
    }
  ]


})(window);
