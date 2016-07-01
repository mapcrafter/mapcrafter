MarkerControl.prototype = new BaseControl("MarkerControl");

function MarkerControl(markers) {
	this.handler = new MarkerHandler(markers);
}

function hasClass(node, clazz) {
	return node.getAttribute("class").split(" ").indexOf(clazz) != -1;
}

function addClass(node, clazz)  {
	var clazzes = node.getAttribute("class").split(" ");
	if (clazzes.indexOf(clazz) == -1) {
		clazzes.push(clazz);
	}
	node.setAttribute("class", clazzes.join(" "));
};

function removeClass(node, clazz) {
	var clazzes = node.getAttribute("class").split(" ");
	while (clazzes.indexOf(clazz) != -1) {
		clazzes.splice(clazzes.indexOf(clazz), 1);
	}
	node.setAttribute("class", clazzes.join(" "));
};

MarkerControl.prototype.create = function(wrapper) {
	var groups = this.handler.getMarkerGroups();

	var checkedClass = "list-group-item-info";
	var listGroup = document.createElement("div");
	listGroup.setAttribute("class", "list-group");

	for (var i = 0; i < groups.length; i++) {
		var group = groups[i][0];
		var groupLabel = groups[i][1];

		var button = document.createElement("button");
		button.setAttribute("type", "button");
		button.setAttribute("class", "list-group-item  " + checkedClass);
		button.setAttribute("data-group", group);
		button.innerHTML = "<span class='badge'>17</span> <span class='right-padding'>" + groupLabel + "</span>";
		button.addEventListener("click", function() {
			var checked = hasClass(this, checkedClass);
			var group = this.getAttribute("data-group");
			self.handler.show(group, !checked);
			
			if (checked) {
				removeClass(this, checkedClass);
			} else {
				addClass(this, checkedClass);
			}
		});

		listGroup.appendChild(button);
	}

	var buttonShowAll = document.createElement("button");
	buttonShowAll.setAttribute("type", "buttonShowAll");
	buttonShowAll.setAttribute("class", "list-group-item");
	buttonShowAll.innerHTML = "Show all";
	buttonShowAll.addEventListener("click", function(handler) {
		return function() {
			for (var i = 0; i < listGroup.childNodes.length - 2; i++) {
				var button = listGroup.childNodes[i];
				var group = button.getAttribute("data-group");
				handler.show(group, true);
				addClass(button, checkedClass);
			}
		}
	}(this.handler));
	
	var buttonHideAll = document.createElement("button");
	buttonHideAll.setAttribute("type", "buttonHideAll");
	buttonHideAll.setAttribute("class", "list-group-item");
	buttonHideAll.innerHTML = "Hide all";
	buttonHideAll.addEventListener("click", function(handler) {
		return function() {
			for (var i = 0; i < listGroup.childNodes.length - 2; i++) {
				var button = listGroup.childNodes[i];
				var group = button.getAttribute("data-group");
				handler.show(group, false);
				removeClass(button, checkedClass);
			}
		}
	}(this.handler));
	
	listGroup.appendChild(buttonShowAll);
	listGroup.appendChild(buttonHideAll);

	var heading = document.createElement("div");
	heading.setAttribute("class", "panel-heading");
	heading.innerHTML = "Markers";

	wrapper.appendChild(heading);
	wrapper.appendChild(listGroup);
};

MarkerControl.prototype.getHandler = function() {
	return this.handler;
};

MarkerControl.prototype.getName = function() {
	return 'marker';
};

MarkerControl.prototype.usePanelWrapper = function() {
	return true;
}

