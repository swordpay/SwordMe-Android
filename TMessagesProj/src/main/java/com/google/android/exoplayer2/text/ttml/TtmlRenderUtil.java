/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.google.android.exoplayer2.text.ttml;

import static com.google.android.exoplayer2.util.Assertions.checkNotNull;

import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.text.style.AbsoluteSizeSpan;
import android.text.style.BackgroundColorSpan;
import android.text.style.ForegroundColorSpan;
import android.text.style.RelativeSizeSpan;
import android.text.style.StrikethroughSpan;
import android.text.style.StyleSpan;
import android.text.style.TypefaceSpan;
import android.text.style.UnderlineSpan;

import androidx.annotation.Nullable;

import com.google.android.exoplayer2.text.Cue;
import com.google.android.exoplayer2.text.span.HorizontalTextInVerticalContextSpan;
import com.google.android.exoplayer2.text.span.RubySpan;
import com.google.android.exoplayer2.text.span.SpanUtil;
import com.google.android.exoplayer2.text.span.TextAnnotation;
import com.google.android.exoplayer2.text.span.TextEmphasisSpan;
import com.google.android.exoplayer2.util.Log;
import com.google.android.exoplayer2.util.Util;

import java.util.ArrayDeque;
import java.util.Deque;
import java.util.Map;

/** Package internal utility class to render styled <code>TtmlNode</code>s. */
/* package */ final class TtmlRenderUtil {

  private static final String TAG = "TtmlRenderUtil";

  @Nullable
  public static TtmlStyle resolveStyle(
      @Nullable TtmlStyle style, @Nullable String[] styleIds, Map<String, TtmlStyle> globalStyles) {
    if (style == null) {
      if (styleIds == null) {

        return null;
      } else if (styleIds.length == 1) {

        return globalStyles.get(styleIds[0]);
      } else if (styleIds.length > 1) {

        TtmlStyle chainedStyle = new TtmlStyle();
        for (String id : styleIds) {
          chainedStyle.chain(globalStyles.get(id));
        }
        return chainedStyle;
      }
    } else /* style != null */ {
      if (styleIds != null && styleIds.length == 1) {

        return style.chain(globalStyles.get(styleIds[0]));
      } else if (styleIds != null && styleIds.length > 1) {

        for (String id : styleIds) {
          style.chain(globalStyles.get(id));
        }
        return style;
      }
    }

    return style;
  }

  public static void applyStylesToSpan(
      Spannable builder,
      int start,
      int end,
      TtmlStyle style,
      @Nullable TtmlNode parent,
      Map<String, TtmlStyle> globalStyles,
      @Cue.VerticalType int verticalType) {

    if (style.getStyle() != TtmlStyle.UNSPECIFIED) {
      builder.setSpan(
          new StyleSpan(style.getStyle()), start, end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
    }
    if (style.isLinethrough()) {
      builder.setSpan(new StrikethroughSpan(), start, end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
    }
    if (style.isUnderline()) {
      builder.setSpan(new UnderlineSpan(), start, end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
    }
    if (style.hasFontColor()) {
      SpanUtil.addOrReplaceSpan(
          builder,
          new ForegroundColorSpan(style.getFontColor()),
          start,
          end,
          Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
    }
    if (style.hasBackgroundColor()) {
      SpanUtil.addOrReplaceSpan(
          builder,
          new BackgroundColorSpan(style.getBackgroundColor()),
          start,
          end,
          Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
    }
    if (style.getFontFamily() != null) {
      SpanUtil.addOrReplaceSpan(
          builder,
          new TypefaceSpan(style.getFontFamily()),
          start,
          end,
          Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
    }
    if (style.getTextEmphasis() != null) {
      TextEmphasis textEmphasis = checkNotNull(style.getTextEmphasis());
      @TextEmphasisSpan.MarkShape int markShape;
      @TextEmphasisSpan.MarkFill int markFill;
      if (textEmphasis.markShape == TextEmphasis.MARK_SHAPE_AUTO) {



        markShape =
            (verticalType == Cue.VERTICAL_TYPE_LR || verticalType == Cue.VERTICAL_TYPE_RL)
                ? TextEmphasisSpan.MARK_SHAPE_SESAME
                : TextEmphasisSpan.MARK_SHAPE_CIRCLE;
        markFill = TextEmphasisSpan.MARK_FILL_FILLED;
      } else {
        markShape = textEmphasis.markShape;
        markFill = textEmphasis.markFill;
      }

      @TextEmphasis.Position int position;
      if (textEmphasis.position == TextEmphasis.POSITION_OUTSIDE) {


        position = TextAnnotation.POSITION_BEFORE;
      } else {
        position = textEmphasis.position;
      }

      SpanUtil.addOrReplaceSpan(
          builder,
          new TextEmphasisSpan(markShape, markFill, position),
          start,
          end,
          Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
    }
    switch (style.getRubyType()) {
      case TtmlStyle.RUBY_TYPE_BASE:

        @Nullable TtmlNode containerNode = findRubyContainerNode(parent, globalStyles);
        if (containerNode == null) {

          break;
        }
        @Nullable TtmlNode textNode = findRubyTextNode(containerNode, globalStyles);
        if (textNode == null) {

          break;
        }
        String rubyText;
        if (textNode.getChildCount() == 1 && textNode.getChild(0).text != null) {
          rubyText = Util.castNonNull(textNode.getChild(0).text);
        } else {
          Log.i(TAG, "Skipping rubyText node without exactly one text child.");
          break;
        }

        @Nullable
        TtmlStyle textStyle = resolveStyle(textNode.style, textNode.getStyleIds(), globalStyles);

        @TextAnnotation.Position
        int rubyPosition =
            textStyle != null ? textStyle.getRubyPosition() : TextAnnotation.POSITION_UNKNOWN;

        if (rubyPosition == TextAnnotation.POSITION_UNKNOWN) {

          @Nullable
          TtmlStyle containerStyle =
              resolveStyle(containerNode.style, containerNode.getStyleIds(), globalStyles);
          rubyPosition = containerStyle != null ? containerStyle.getRubyPosition() : rubyPosition;
        }

        builder.setSpan(
            new RubySpan(rubyText, rubyPosition), start, end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        break;
      case TtmlStyle.RUBY_TYPE_DELIMITER:


      case TtmlStyle.RUBY_TYPE_TEXT:




        builder.setSpan(new DeleteTextSpan(), start, end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        break;
      case TtmlStyle.RUBY_TYPE_CONTAINER:
      case TtmlStyle.UNSPECIFIED:
      default:

        break;
    }
    if (style.getTextCombine()) {
      SpanUtil.addOrReplaceSpan(
          builder,
          new HorizontalTextInVerticalContextSpan(),
          start,
          end,
          Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
    }
    switch (style.getFontSizeUnit()) {
      case TtmlStyle.FONT_SIZE_UNIT_PIXEL:
        SpanUtil.addOrReplaceSpan(
            builder,
            new AbsoluteSizeSpan((int) style.getFontSize(), true),
            start,
            end,
            Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        break;
      case TtmlStyle.FONT_SIZE_UNIT_EM:
        SpanUtil.addOrReplaceSpan(
            builder,
            new RelativeSizeSpan(style.getFontSize()),
            start,
            end,
            Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        break;
      case TtmlStyle.FONT_SIZE_UNIT_PERCENT:
        SpanUtil.addOrReplaceSpan(
            builder,
            new RelativeSizeSpan(style.getFontSize() / 100),
            start,
            end,
            Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        break;
      case TtmlStyle.UNSPECIFIED:

        break;
    }
  }

  @Nullable
  private static TtmlNode findRubyTextNode(
      TtmlNode rubyContainerNode, Map<String, TtmlStyle> globalStyles) {
    Deque<TtmlNode> childNodesStack = new ArrayDeque<>();
    childNodesStack.push(rubyContainerNode);
    while (!childNodesStack.isEmpty()) {
      TtmlNode childNode = childNodesStack.pop();
      @Nullable
      TtmlStyle style = resolveStyle(childNode.style, childNode.getStyleIds(), globalStyles);
      if (style != null && style.getRubyType() == TtmlStyle.RUBY_TYPE_TEXT) {
        return childNode;
      }
      for (int i = childNode.getChildCount() - 1; i >= 0; i--) {
        childNodesStack.push(childNode.getChild(i));
      }
    }

    return null;
  }

  @Nullable
  private static TtmlNode findRubyContainerNode(
      @Nullable TtmlNode node, Map<String, TtmlStyle> globalStyles) {
    while (node != null) {
      @Nullable TtmlStyle style = resolveStyle(node.style, node.getStyleIds(), globalStyles);
      if (style != null && style.getRubyType() == TtmlStyle.RUBY_TYPE_CONTAINER) {
        return node;
      }
      node = node.parent;
    }
    return null;
  }

  /**
   * Called when the end of a paragraph is encountered. Adds a newline if there are one or more
   * non-space characters since the previous newline.
   *
   * @param builder The builder.
   */
  /* package */ static void endParagraph(SpannableStringBuilder builder) {
    int position = builder.length() - 1;
    while (position >= 0 && builder.charAt(position) == ' ') {
      position--;
    }
    if (position >= 0 && builder.charAt(position) != '\n') {
      builder.append('\n');
    }
  }

  /**
   * Applies the appropriate space policy to the given text element.
   *
   * @param in The text element to which the policy should be applied.
   * @return The result of applying the policy to the text element.
   */
  /* package */ static String applyTextElementSpacePolicy(String in) {

    String out = in.replaceAll("\r\n", "\n");


    out = out.replaceAll(" *\n *", "\n");

    out = out.replaceAll("\n", " ");

    out = out.replaceAll("[ \t\\x0B\f\r]+", " ");
    return out;
  }

  private TtmlRenderUtil() {}
}
