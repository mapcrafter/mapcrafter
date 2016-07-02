var Util = {
	hasClass: function(node, clazz) {
		return node.getAttribute("class").split(" ").indexOf(clazz) != -1;
	},

	addClass: function(node, clazz)  {
		var clazzes = node.getAttribute("class").split(" ");
		if (clazzes.indexOf(clazz) == -1) {
			clazzes.push(clazz);
		}
		node.setAttribute("class", clazzes.join(" "));
	},

	removeClass: function(node, clazz) {
		var clazzes = node.getAttribute("class").split(" ");
		while (clazzes.indexOf(clazz) != -1) {
			clazzes.splice(clazzes.indexOf(clazz), 1);
		}
		node.setAttribute("class", clazzes.join(" "));
	},

	createPanelHeader: function(title) {
		var header = document.createElement("div");
		header.setAttribute("class", "panel-heading");
		header.innerHTML = title;
		return header;
	},
};

