MarkerControl.prototype = new BaseControl("MarkerControl");

function MarkerControl(markers) {
	this.handler = new MarkerHandler(this, markers);
	this.buttons = [];
}

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
		button.addEventListener("click", function(handler) {
			return function() {
				var checked = Util.hasClass(this, checkedClass);
				var group = this.getAttribute("data-group");
				handler.show(group, !checked);
				
				if (checked) {
					Util.removeClass(this, checkedClass);
				} else {
					Util.addClass(this, checkedClass);
				}
			}
		}(this.handler));

		listGroup.appendChild(button);
		this.buttons.push(button);
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
				Util.addClass(button, checkedClass);
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
				Util.removeClass(button, checkedClass);
			}
		}
	}(this.handler));
	
	listGroup.appendChild(buttonShowAll);
	listGroup.appendChild(buttonHideAll);

	wrapper.appendChild(Util.createPanelHeader("Markers"));
	wrapper.appendChild(listGroup);
};

// hmm this is a not-so-nice hack
MarkerControl.prototype.uncheckGroup = function(group) {
	for (var i = 0; i < this.buttons.length; i++) {
		var button = this.buttons[i];
		if (button.getAttribute("data-group") == group) {
			Util.removeClass(button, "list-group-item-info");
		}
	}
}

MarkerControl.prototype.getHandler = function() {
	return this.handler;
};

MarkerControl.prototype.getName = function() {
	return 'marker';
};

MarkerControl.prototype.usePanelWrapper = function() {
	return true;
}

