#ifndef LVREND_VERT_COVER_H_INCLUDED
#define LVREND_VERT_COVER_H_INCLUDED

class ldomNode;
class LFormattedText;

// The opening image-only EPUB fragment is a cover only when its image matches
// the document's declared cover. Other illustrations retain normal page flow.
bool isVerticalCoverFragment(ldomNode *node);

// Establish the isolated cover's cross-axis region as formatter input. Every
// consumer (layout, painting, selection, cache rebuild) then gets the same lines.
void configureVerticalCoverFormatting(ldomNode *node, LFormattedText *text);

#endif
