#include "layout.h"

#include "widget.h"

CLayout::CLayout(CWidget *parent)
{
	_parent = parent;
}

void CLayout::addWidget(CWidget *widget)
{
	_widgets.push_back(widget);
	setSize(_size.width(), _size.height());
}

void CLayout::hide()
{
	for(vector<CWidget *>::const_iterator i = _widgets.begin(); i != _widgets.end(); i++)
		(*i)->hide();
}

void CLayout::show()
{
	for(vector<CWidget *>::const_iterator i = _widgets.begin(); i != _widgets.end(); i++)
		(*i)->show();
}

void CLayout::update()
{
	for(vector<CWidget *>::const_iterator i = _widgets.begin(); i != _widgets.end(); i++)
		(*i)->update();
}

int CLayout::count()
{
	return _widgets.size();
}

void CLayout::setSize(uint width, uint height)
{
	_size.set(width, height);
	
	if(width > 0 && height > 0)
		recursiveResize(0, _widgets.size() - 1, width, height, _pos.x(), _pos.y());
}

void CLayout::setPosition(uint x, uint y)
{
	if(x + _size.width() <= uint(COLS) && y + _size.height() <= uint(LINES))
	{
		uint oldx = _pos.x(), oldy = _pos.y();

		_pos.set(x, y);

		for(uint i = 0; i < _widgets.size(); i++)
		{
			if(_widgets[i]->pos().x() < oldx || _widgets[i]->pos().y() < oldy)
				_widgets[i]->setPosition(x, y);
			else
				_widgets[i]->setPosition(x + _widgets[i]->pos().x() - oldx,
										 y + _widgets[i]->pos().y() - oldy);
		}
	}
}

void CLayout::recursiveResize(uint from, uint to, uint width, uint height, uint x, uint y)
{

}

CVBoxLayout::CVBoxLayout(CWidget *parent)
	: CLayout(parent)
{
}

void CVBoxLayout::recursiveResize(uint from, uint to, uint width, uint height, uint x, uint y)
{
	if(to - from == 0) // if only one element
	{
		_widgets[0]->setSize(width, height);
		return;
	}	
	else if(to - from == 1) // if only two elements
	{
		if(_widgets[to]->size().fixed())
		{
			// resize #2 fixed
			uint nh = _widgets[to]->size().height();

			if(nh > height)
				nh = height / 2; // space for other widgets

			_widgets[to]->setFixedSize(width, nh);
			_widgets[to]->setPosition(x, y + height - nh);

			// resize #1 not fixed
			_widgets[from]->setSize(width, height - nh);
			_widgets[from]->setPosition(x, y);
		}
		else
		{
			_widgets[from]->setSize(width, height / 2);
			_widgets[from]->setPosition(x, y);
			_widgets[to]->setSize(width, height - _widgets[from]->size().height());
			_widgets[to]->setPosition(x, y + height / 2);
		}
		return;
	}

	bool k = false; // no fixed size found
	for (uint i = from; i <= to; i++)
	{
		if(_widgets[i]->size().fixed())
		{
			uint nh = _widgets[i]->size().height();

			// fixed
			// set size
			// limits: width, height
			
			if(nh > height)
				nh = height / 2; // space for other widgets

			_widgets[i]->setFixedSize(width, nh);
			_widgets[i]->setPosition(x, y);

			// i+1..to
			recursiveResize(i + 1, to, width, height - nh, x, y + nh);
			break;
		}
		else
		{
			// not fixed
			for(uint j = i + 1; j <= to && !k; j++)
			{
				if(_widgets[i]->size().fixed())
				{
					k = true; // found
				}
				else if(k)
				{
					// i..j-1
					recursiveResize(i, j - 1, width, height, x, y);
					break;
				}
			}
			if(!k) // did not find any fixed
			{
				// set size, i..to
				// limits: width, height / (to - i + 1)

				for(uint j = i; j <= to; j++)
				{
					_widgets[j]->setSize(width, height / (to - i + 1));
					_widgets[j]->setPosition(x, y + j * height / (to - i + 1));
				}

				break;
			}
			k = false; // reset
		}
	}
}

CHBoxLayout::CHBoxLayout(CWidget *parent)
	: CLayout(parent)
{
}

void CHBoxLayout::recursiveResize(uint from, uint to, uint width, uint height, uint x, uint y)
{
	if(to - from == 0) // if only one element
	{
		_widgets[0]->setSize(width, height);
		return;
	}	
	else if(to - from == 1) // if only two elements
	{
		if(_widgets[from]->size().fixed() && _widgets[to]->size().fixed())
		{
			// resize #1 fixed
			uint nw = _widgets[from]->size().width();

			if(nw > width)
				nw = width / 2; // space for other widgets

			_widgets[from]->setFixedSize(nw, height);
			_widgets[from]->setPosition(x, y);

			// resize #2 fixed
			uint nw2 = _widgets[to]->size().width();

			if(nw2 > width - nw)
				nw2 = width - nw;

			_widgets[to]->setFixedSize(nw2, height);
			_widgets[to]->setPosition(x + nw, y);
		}
		else if(_widgets[to]->size().fixed())
		{
			// resize #2 fixed
			uint nw = _widgets[to]->size().width();

			if(nw > width)
				nw = width / 2; // space for other widgets

			_widgets[to]->setFixedSize(nw, height);
			_widgets[to]->setPosition(width - nw, y);

			// resize #1 not fixed
			_widgets[from]->setSize(width - nw, height);
			_widgets[from]->setPosition(x, y);
		}
		else if(_widgets[from]->size().fixed())
		{
			// resize #1 fixed
			uint nw = _widgets[from]->size().width();

			if(nw > width)
				nw = width / 2; // space for other widgets

			_widgets[from]->setFixedSize(nw, height);
			_widgets[from]->setPosition(x, y);

			// resize #2 not fixed
			_widgets[to]->setSize(width - nw, height);
			_widgets[to]->setPosition(x + nw, y);
		}
		else
		{
			_widgets[from]->setSize(width / 2, height);
			_widgets[from]->setPosition(x, y);
			_widgets[to]->setSize(width - _widgets[from]->size().width(), height);
			_widgets[to]->setPosition(x + width / 2, y);
		}
		return;
	}

	bool k = false; // no fixed size found
	for (uint i = from; i <= to; i++)
	{
		if(_widgets[i]->size().fixed())
		{
			uint nw = _widgets[i]->size().width();

			// fixed
			// set size
			// limits: width, height
			
			if(nw > width)
				nw = width / 2; // space for other widgets

			_widgets[i]->setFixedSize(nw, height);
			_widgets[i]->setPosition(x, y);

			// i+1..to
			recursiveResize(i + 1, to, width - nw, height, x + nw, y);
			break;
		}
		else
		{
			// not fixed
			for(uint j = i + 1; j <= to && !k; j++)
			{
				if(_widgets[i]->size().fixed())
				{
					k = true; // found
				}
				else if(k)
				{
					// i..j-1
					recursiveResize(i, j - 1, width, height, x, y);
					break;
				}
			}
			if(!k) // did not find any fixed
			{
				// set size, i..to
				// limits: width, height / (to - i + 1)

				for(uint j = i; j <= to; j++)
				{
					_widgets[j]->setSize(width / (to - i + 1), height);
					_widgets[j]->setPosition(x + j * width / (to - i + 1), y);
				}

				break;
			}
			k = false; // reset
		}
	}
}