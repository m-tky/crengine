#include "../include/lvrend_vert_cover.h"
#include "../include/lvtinydom.h"
#include "../include/fb2def.h"
#include "../include/lvtextfm.h"

static bool containsOnlyCoverImage(ldomNode *node, const lString32 &cover, int &images)
{
    if ( node->isText() )
        return node->isWhitespaceText();
    lUInt16 id = node->getNodeId();
    if ( id == el_stylesheet || id == el_script )
        return true;
    if ( id == el_img || id == el_image ) {
        lString32 src = node->getAttributeValue(id == el_img ? attr_src : attr_href);
        if ( src != cover )
            return false;
        images++;
        return true;
    }
    for ( int i=0; i<node->getChildCount(); i++ ) {
        if ( !containsOnlyCoverImage(node->getChildNode(i), cover, images) )
            return false;
    }
    return true;
}

bool isVerticalCoverFragment(ldomNode *node)
{
    if ( !node || node->getNodeId() != el_DocFragment )
        return false;
    ldomNode *parent = node->getParentNode();
    if ( !parent )
        return false;
    for ( int i=0; i<parent->getChildCount(); i++ ) {
        ldomNode *sibling = parent->getChildNode(i);
        if ( sibling == node )
            break;
        if ( sibling->getNodeId() == el_DocFragment )
            return false;
    }
    lString32 cover = node->getDocument()->getProps()->getStringDef(DOC_PROP_COVER_FILE);
    if ( cover.empty() )
        return false;
    int images = 0;
    return containsOnlyCoverImage(node, cover, images) && images == 1;
}

void configureVerticalCoverFormatting(ldomNode *node, LFormattedText *text)
{
    ldomNode *fragment = node;
    while ( fragment && fragment->getNodeId() != el_DocFragment )
        fragment = fragment->getParentNode();
    if ( !isVerticalCoverFragment(fragment) )
        return;
    // A cover owns the full page cross-axis region, less surrounding boxes.
    // Set this before Format(), not by changing formatted lines afterwards:
    // the normal single-image formatter and drawer already center within it.
    int extent = node->getDocument()->getPageWidth()
            - node->getSurroundingAddedHeight(false);
    formatted_text_fragment_t *buffer = text->GetBuffer();
    if ( extent > buffer->strut_height ) {
        int baseline = buffer->strut_baseline + (extent - buffer->strut_height) / 2;
        text->setStrut(extent, baseline);
    }
}
