//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAPI/HistoryView.h"

namespace Mantid
{
namespace API
{

HistoryView::HistoryView(const WorkspaceHistory& wsHist)
  : m_wsHist(wsHist), m_historyItems()
{
  //add all of the top level algorithms to the view by default
  const auto algorithms = wsHist.getAlgorithmHistories();
  AlgorithmHistories::const_iterator iter = algorithms.begin();
  for ( ; iter != algorithms.end(); ++iter)
  {
    HistoryItem item(*iter);
    m_historyItems.push_back(item);
  }
}

/**
 * Unroll an algorithm history to export its child algorithms.
 *
 * This places each of the child algorithm histories into the 
 * HistoryView object. The parent is retained as a marker so we can
 * "roll" the history back up if we want. This method does nothing if
 * the history object has no children
 *
 * @param index :: index of the history object to unroll
 * @throws std::out_of_range if the index is larger than the number of history items.
 */
void HistoryView::unroll(size_t index)
{
  if( index >= m_historyItems.size() )
  {
    throw std::out_of_range("HistoryView::unroll() - Index out of range");
  }

  //advance to the item at the index
  auto it = m_historyItems.begin();
  std::advance (it,index);

  const auto history = it->getAlgorithmHistory();
  const auto childHistories = history->getChildHistories();

  if (!it->isUnrolled() && childHistories.size() > 0)
  {  
    //mark this record as being ignored by the script builder
    it->unrolled(true);
    
    ++it; //move iterator forward to insertion position
    //insert each of the records, in order, at this position
    for (auto childIter = childHistories.begin(); childIter != childHistories.end(); ++childIter)
    {
      HistoryItem item(*childIter);
      m_historyItems.insert(it, item);
    }
  }
}

/**
 * Roll an unrolled algorithm history item and remove its children from the view.
 *
 * This removes each of the child algorithm histories (if any) and marks
 * the parent as being "rolled up". Note that this will recursively "roll up" any child
 * history objects that are also unrolled. This method does nothing if
 * the history object has no children.
 *
 * @param index :: index of the history object to unroll
 * @throws std::out_of_range if the index is larger than the number of history items.
 */
void HistoryView::roll(size_t index)
{
  if( index >= m_historyItems.size() )
  {
    throw std::out_of_range("HistoryView::roll() - Index out of range");
  }

  //advance to the item at the index
  auto it = m_historyItems.begin();
  std::advance (it,index);

  // the number of records after this position
  const size_t numChildren = it->numberOfChildren();
  if (it->isUnrolled() && numChildren > 0)
  {  
    //mark this record as not being ignored by the script builder
    it->unrolled(false);
    ++it; //move to first child

    //remove each of the children from the list
    for (size_t i = 0; i < numChildren; ++i)
    {
      //check if our children are unrolled and 
      //roll them back up if so.
      if(it->isUnrolled())
      {
        roll(index+1);
      }
      //Then just remove the item from the list
      it = m_historyItems.erase(it);
    }
  }
}

const std::vector<HistoryItem> HistoryView::getAlgorithmsList()
{
  std::vector<HistoryItem> histories;
  histories.reserve(size());
  std::copy(m_historyItems.cbegin(), m_historyItems.cend(), std::back_inserter( histories ));
  return histories;
}

} // namespace API
} // namespace Mantid
