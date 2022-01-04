<template>
    <div class="slider-wraper"
        v-on:mouseup="onEnd"
        v-on:mousecancel="onEnd"
        v-on:mouseleave="onEnd"
        v-on:mousemove="onMove"
    >
        <div class="slider" ref="slider">
            <div class="slider-bar" :style="barStyle"></div>
            <div class="slider-button-wraper" 
                :style="buttonWraperStyle"
                v-on:mouseenter="onEnter"
                v-on:mouseleave="onLeave"
                v-on:mousedown="onStart"
            >
                <div :class="tooltipClass">{{computedValue}}</div>
                <div class="slider-button"></div>
            </div>
        </div>
    </div>
</template>
<script>
function getBoundingClientRect(element/*: HTMLElement*/) {
    if (element.getBoundingClientRect) {
        const rect = element.getBoundingClientRect();
        return {
            top: rect.top,
            left: rect.left,
            right: rect.right,
            bottom: rect.bottom,
        }
    }
    return { top: 0, left: 0, right: 0, bottom: 0 }
}
function getBoundingClientRectSize(element/*: HTMLElement*/) {
    const rect = getBoundingClientRect(element);
    return {
        width: rect.right - rect.left,
        height: rect.bottom - rect.top,
    }
}

export default {
    props: ['min', 'max'],
    components: {
    },
    data() {
        return {
            tooltip: false,
            value: 50,
            sliderSize: {
                width: 0,
                height: 0,
            },
            started: false,
        } 
    },
    computed: {
        computedValue() {
            const min = this.min ? this.min : 0;
            const max = this.max ? this.max : 100;
            const range = max - min;
            return Math.round(range * this.value / 100) + min;
        },
        tooltipClass() {
            return this.tooltip ? 'slider-button-tooltip' : 'slider-button-tooltip hide';
        },
        barStyle() {
            return "width:" + this.value + "%";
        },
        buttonWraperStyle() {
            return "left:" + this.value + "%";
        },
    },
    methods: {
        onEnter() {
            this.tooltip = true;
        },
        onLeave() {
            this.tooltip = false;
        },
        onStart() {
            this.started = true;
        },
        onEnd() {
            this.started = false;
        },
        onMove(e) {
            if (!this.started) {
                return;
            }
            if (this.sliderSize.width == 0 && this.$refs['slider']) {
                this.sliderSize = getBoundingClientRectSize(this.$refs['slider']);
            }
            if (this.sliderSize.width == 0) {
                return;
            }

            // const value = e.movementX > 0 ? this.value + 1 : this.value - 1;
            // Log.warn("onMove", e, e.movementX);
            // TODO add touch handle
            // WARNING not handle touch
            const value = this.value + e.movementX / this.sliderSize.width * 100;
            if (value > 100 || value < 0) {
                return;
            }
            this.value = value;
            this.$emit('change', this.computedValue);
        },
    },
    mounted() {
        console.log("slider size", getBoundingClientRectSize(this.$refs['slider']));
        if (this.$refs['slider']) {
            this.sliderSize = getBoundingClientRectSize(this.$refs['slider']);
        }
    }
}
</script>
<style scoped>
.slider-wraper {
    padding: 16px 0 16px 0;
    width: 100%;
}

.slider {
    width: 100%;
    height: 6px;
    background-color: #E4E7ED;
    border-radius: 3px;
    position: relative;
    cursor: pointer;
    vertical-align: middle;
}

.slider-bar {
    left: 0%;
    height: 6px;
    background-color: rgba(18, 37, 95, 1);
    border-top-left-radius: 3px;
    border-bottom-left-radius: 3px;
    position: absolute;
}
.slider-button-wraper {
    height: 36px;
    width: 36px;
    position: absolute;
    z-index: 1001;
    top: -15px;
    transform: translateX(-50%);
    background-color: transparent;
    text-align: center;
    user-select: none;
    line-height: normal;
}

.slider-button {
    width: 16px;
    height: 16px;
    border: solid 2px rgba(18, 37, 95, 1);
    background-color: #fff;
    border-radius: 50%;
    transition: .2s;
    user-select: none;
    vertical-align: middle;
    display: inline-block;
}
.slider-button:hover {
    cursor: grab;
    transform: scale(1.2);
}

.slider-button-wraper::after {
    display: inline-block;
    content: "";
    height: 100%;
    vertical-align: middle;
}
.slider-button-tooltip {
    position: absolute;
    width: 36px;
    background-color: rgba(18, 37, 95, 1);
    top: -12px;
    height: 18px;
    line-height: 18px;
    font-size: 12px;
    color:#fff;
    border-radius: 2px;
    transition: all ease .5s;
    opacity: 1;
}
.slider-button-tooltip.hide {
    opacity: 0;
}

.slider::before,.slider::after {
    display: table;
    content: "";
}
.slider::after {
    clear: both;
}
</style>