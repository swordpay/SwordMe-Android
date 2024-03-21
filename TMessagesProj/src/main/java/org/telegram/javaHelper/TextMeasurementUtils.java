package org.telegram.javaHelper;

import android.os.Build;
import android.text.Layout;
import android.text.StaticLayout;
import android.text.TextDirectionHeuristic;
import android.text.TextPaint;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;

public class TextMeasurementUtils {

    public static List<CharSequence> getTextLines(CharSequence text, TextMeasurementParams params) {
        StaticLayout layout;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            StaticLayout.Builder builder = StaticLayout.Builder
                    .obtain(text, 0, text.length(), params.textPaint, params.width)
                    .setAlignment(params.alignment)
                    .setLineSpacing(params.lineSpacingExtra, params.lineSpacingMultiplier)
                    .setIncludePad(params.includeFontPadding)
                    .setBreakStrategy(params.breakStrategy)
                    .setHyphenationFrequency(params.hyphenationFrequency);
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                builder.setJustificationMode(params.justificationMode);
            }
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
                builder.setUseLineSpacingFromFallbacks(params.useFallbackLineSpacing);
            }
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                builder.setTextDirection((TextDirectionHeuristic) params.textDirectionHeuristic);
            }
            layout = builder.build();
        } else {
            layout = new StaticLayout(
                    text,
                    params.textPaint,
                    params.width,
                    params.alignment,
                    params.lineSpacingMultiplier,
                    params.lineSpacingExtra,
                    params.includeFontPadding);
        }
        List<CharSequence> result = new ArrayList<>();
        for (int i = 0; i < layout.getLineCount(); i++) {
            result.add(layout.getText().subSequence(layout.getLineStart(i), layout.getLineEnd(i)));
        }
        return result;
    }

    /**
     * The text measurement parameters
     */
    public static class TextMeasurementParams {
        public final TextPaint textPaint;
        public final Layout.Alignment alignment;
        public final float lineSpacingExtra;
        public final float lineSpacingMultiplier;
        public final boolean includeFontPadding;
        public final int breakStrategy;
        public final int hyphenationFrequency;
        public final int justificationMode;
        public final boolean useFallbackLineSpacing;
        public final Object textDirectionHeuristic;
        public final int width;

        private TextMeasurementParams(Builder builder) {
            textPaint = requireNonNull(builder.textPaint);
            alignment = requireNonNull(builder.alignment);
            lineSpacingExtra = builder.lineSpacingExtra;
            lineSpacingMultiplier = builder.lineSpacingMultiplier;
            includeFontPadding = builder.includeFontPadding;
            breakStrategy = builder.breakStrategy;
            hyphenationFrequency = builder.hyphenationFrequency;
            justificationMode = builder.justificationMode;
            useFallbackLineSpacing = builder.useFallbackLineSpacing;
            textDirectionHeuristic = builder.textDirectionHeuristic;
            width = builder.width;
        }


        public static final class Builder {
            private TextPaint textPaint;
            private Layout.Alignment alignment;
            private float lineSpacingExtra;
            private float lineSpacingMultiplier = 1.0f;
            private boolean includeFontPadding = true;
            private int breakStrategy;
            private int hyphenationFrequency;
            private int justificationMode;
            private boolean useFallbackLineSpacing;
            private Object textDirectionHeuristic;
            private int width;

            public Builder() {
            }

            public Builder(TextMeasurementParams copy) {
                this.textPaint = copy.textPaint;
                this.alignment = copy.alignment;
                this.lineSpacingExtra = copy.lineSpacingExtra;
                this.lineSpacingMultiplier = copy.lineSpacingMultiplier;
                this.includeFontPadding = copy.includeFontPadding;
                this.breakStrategy = copy.breakStrategy;
                this.hyphenationFrequency = copy.hyphenationFrequency;
                this.justificationMode = copy.justificationMode;
                this.useFallbackLineSpacing = copy.useFallbackLineSpacing;
                this.textDirectionHeuristic = copy.textDirectionHeuristic;
                this.width = copy.width;
            }

            public static Builder from(TextView view) {
                Layout layout = view.getLayout();
                Builder result = new Builder()
                        .textPaint(layout.getPaint())
                        .alignment(layout.getAlignment())
                        .width(view.getWidth() -
                                view.getCompoundPaddingLeft() - view.getCompoundPaddingRight());
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
                    result.lineSpacingExtra(view.getLineSpacingExtra())
                            .lineSpacingMultiplier(view.getLineSpacingMultiplier())
                            .includeFontPadding(view.getIncludeFontPadding());
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                        result.breakStrategy(view.getBreakStrategy())
                                .hyphenationFrequency(view.getHyphenationFrequency());
                    }
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                        result.justificationMode(view.getJustificationMode());
                    }
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
                        result.useFallbackLineSpacing(view.isFallbackLineSpacing());
                    }
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                        result.textDirectionHeuristic(view.getTextDirectionHeuristic());
                    }
                }
                return result;
            }

            public Builder textPaint(TextPaint val) {
                textPaint = val;
                return this;
            }

            public Builder alignment(Layout.Alignment val) {
                alignment = val;
                return this;
            }

            public Builder lineSpacingExtra(float val) {
                lineSpacingExtra = val;
                return this;
            }

            public Builder lineSpacingMultiplier(float val) {
                lineSpacingMultiplier = val;
                return this;
            }

            public Builder includeFontPadding(boolean val) {
                includeFontPadding = val;
                return this;
            }

            public Builder breakStrategy(int val) {
                breakStrategy = val;
                return this;
            }

            public Builder hyphenationFrequency(int val) {
                hyphenationFrequency = val;
                return this;
            }

            public Builder justificationMode(int val) {
                justificationMode = val;
                return this;
            }

            public Builder useFallbackLineSpacing(boolean val) {
                useFallbackLineSpacing = val;
                return this;
            }

            public Builder textDirectionHeuristic(Object val) {
                textDirectionHeuristic = val;
                return this;
            }

            public Builder width(int val) {
                width = val;
                return this;
            }

            public TextMeasurementParams build() {
                return new TextMeasurementParams(this);
            }
        }
    }

    public static <T> T requireNonNull(T obj) {
        if (obj == null)
            throw new NullPointerException();
        return obj;
    }
}