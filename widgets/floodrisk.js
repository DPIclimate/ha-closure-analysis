
async function GetFloodRisk() {
	var opts = {
		angle: 0.0, // The span of the gauge arc
		lineWidth: 0.44, // The line thickness
		radiusScale: 1, // Relative radius
		pointer: {
			length: 0.6, // // Relative to gauge radius
			strokeWidth: 0.05, // The thickness
			color: '#000000' // Fill color
		},
		limitMax: 100,     // If false, max value increases automatically if value > maxValue
		limitMin: 0,     // If true, the min value of the gauge will be fixed
		staticZones: [
			   {strokeStyle: "#CAF0F8", min: 0, max: 20},
			   {strokeStyle: "#90E0EF", min: 21, max: 40},
			   {strokeStyle: "#00B4D8", min: 41, max: 60},
			   {strokeStyle: "#03045E", min: 61, max: 100},
		],

		strokeColor: '#E0E0E0',  // to see which ones work best for you
		generateGradient: true,
		highDpiSupport: true,     // High resolution support
	};

	return opts;
}

