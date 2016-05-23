(function (window) {
	window.paymentTypesDefinitions = [
		{
			name: 'Cash',
			id: 'cash',
			fee: {
				title: 'zero',
				value: 0,
				unit: 'cash'
			}
		},
		{
			name: 'Credit card',
			id: 'card',
			fee: {
				title: '3%',
				value: 3,
				unit: 'percent'
			}
		},
		{
			name: 'Check',
			id: 'check',
			fee: {
				title: '$3',
				value: 3,
				unit: 'cash'
			}
		}
	];
})(window);
